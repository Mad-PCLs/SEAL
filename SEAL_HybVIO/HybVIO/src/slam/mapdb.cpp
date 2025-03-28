#include "mapdb.hpp"
#include "../util/logging.hpp"
#include "../odometry/util.hpp"

namespace {
Eigen::Matrix4d removeZAxisTiltComparedToReference(const Eigen::Matrix4d &poseCW, const Eigen::Matrix4d &refPose) {
    const Eigen::Matrix3d poseRot = refPose.topLeftCorner<3,3>();
    return odometry::util::replacePoseOrientationKeepPosition(
        poseCW,
        poseRot * odometry::util::removeRotationMatrixZTilt(
            poseRot.inverse() *
            poseCW.topLeftCorner<3,3>()));
}

const slam::Pose *findInPoseTrail(const std::vector<slam::Pose> &poseTrail, slam::KfId targetKeyframeId) {
    for (const auto &pose : poseTrail) {
        if (targetKeyframeId == slam::KfId(pose.frameNumber)) {
            return &pose;
        }
    }
    return nullptr;
}

/*void debugPrintPoseTrail(const std::vector<slam::Pose> &poseTrail) {
    std::string poseTrailStr = "";
    for (const auto &p : poseTrail) {
        poseTrailStr += std::to_string(p.frameNumber) + ", ";
    };
    log_debug("current pose trail: %s", poseTrailStr.c_str());
}*/
}

namespace slam {

std::shared_ptr<Keyframe> MapDB::insertNewKeyframeCandidate(
    std::unique_ptr<Keyframe> keyframeUniq,
    bool keyframeDecision,
    const std::vector<slam::Pose> &poseTrail,
    const odometry::ParametersSlam &parameters)
{
    Eigen::Matrix4d pose, smoothPose;
    std::shared_ptr<Keyframe> keyframe = std::move(keyframeUniq);
    Keyframe *previousKf = latestKeyframe();
    // debugPrintPoseTrail(poseTrail);

    smoothPose = poseTrail[0].pose;
    // TODO: to be more accurate, should uncertainty "deltas" over pose trail
    Eigen::Matrix<double, 3, 6> curUncertainty = poseTrail[0].uncertainty * parameters.keyframeCandidateInterval;
    // assert(curUncertainty.squaredNorm() > 1e-20); // matters only if used

    if (prevPoseKfId.v < 0) {
        // First KF (or something bad happened): use odometry pose as-is
        pose = poseTrail[0].pose;
        // Tip: Use this to change direction of SLAM map and debug transform issues.
        // pose.topLeftCorner<3, 3>() = pose.topLeftCorner<3, 3>() * Eigen::AngleAxisd(0.5 * M_PI, Vector3d::UnitZ());
        // pose.block<3, 1>(0, 3) << 4, 1, -2;
    }
    else {
        assert(previousKf);
        Eigen::Matrix4d refPose = prevPose;

        // Could update prevPose to previousKf->poseCW here, but this does not play nice
        // with loop closures or non-keyframes and the practical difference is very small

        if (parameters.useOdometryPoseTrailDelta) {
            // note that this can fail if in certain cases with long
            // stationarity and no new keyframes unless special care is taken
            const auto *prevPoseInPoseTrail = findInPoseTrail(poseTrail, prevPoseKfId);
            if (prevPoseInPoseTrail == nullptr) {
                log_warn("keyframe %d not found in pose trail", prevPoseKfId.v);
            } else {
                assert(KfId(prevPoseInPoseTrail->frameNumber) != keyframe->id);
                smoothPose = poseTrail[0].pose * prevPoseInPoseTrail->pose.inverse() * prevSmoothPose;
                // No need to remove Z-axis tilt here. "smoothPose" should only be used for computing
                // odometry priors between consecutive keyframes but never as an absolute pose
            }
        }

        Eigen::Matrix4d poseTilted = poseTrail[0].pose * prevInputPose.inverse() * refPose;
        if (parameters.removeOdometryTransformZAxisTilt) {
            pose = removeZAxisTiltComparedToReference(poseTilted, smoothPose);
        } else {
            pose = poseTilted;
        }
    }

    keyframe->poseCW = pose;
    keyframe->smoothPoseCW = smoothPose;
    keyframe->uncertainty = prevUncertainty + curUncertainty;

    if (previousKf) {
        keyframe->previousKfId = previousKf->id;
        previousKf->nextKfId = keyframe->id;
    }

    lastKfCandidateId = keyframe->id;
    if (keyframeDecision) {
        lastKfId = keyframe->id;
    }

    keyframes[keyframe->id] = keyframe;
    return keyframe;
}

MapDB::MapDB(const MapDB &mapDB) {
    for (const auto &kfP : mapDB.keyframes) {
        keyframes.emplace(kfP.second->id, std::make_unique<Keyframe>(*kfP.second));
    }

    mapPoints = mapDB.mapPoints;
    trackIdToMapPoint = mapDB.trackIdToMapPoint;
    loopClosureEdges = mapDB.loopClosureEdges;

    prevPose = mapDB.prevPose;
    prevInputPose = mapDB.prevInputPose;
    prevSmoothPose = mapDB.prevSmoothPose;
    prevUncertainty = mapDB.prevUncertainty;

    nextMp = mapDB.nextMp;
    lastKfCandidateId = mapDB.lastKfCandidateId;
    lastKfId = mapDB.lastKfId;
}

MapDB::MapDB(const MapDB &mapDB, const std::set<KfId> &activeKeyframes) {
    std::set<MpId> activeMapPoints;

    for (auto &kfId : activeKeyframes) {
        auto &origKf = mapDB.keyframes.at(kfId);
        auto kf = (
            *keyframes.emplace(origKf->id, std::make_unique<Keyframe>(*origKf)).first
        ).second.get();
        if (kf->nextKfId.v >= 0 && !activeKeyframes.count(kf->nextKfId)) {
            kf->nextKfId = KfId(-1);
        }
        if (kf->previousKfId.v >= 0 && !activeKeyframes.count(kf->previousKfId)) {
            kf->previousKfId = KfId(-1);
        }
        for (auto mpId : kf->mapPoints) {
            if (mpId.v >= 0) {
                activeMapPoints.insert(mpId);
            }
        }
    }

    for (auto &mpId : activeMapPoints) {
        mapPoints.emplace(mpId, MapPoint(mapDB.mapPoints.at(mpId), activeKeyframes));
    }

    std::copy_if(mapDB.trackIdToMapPoint.begin(), mapDB.trackIdToMapPoint.end(), std::inserter(trackIdToMapPoint, trackIdToMapPoint.begin()),
        [activeMapPoints](std::pair<TrackId, MpId> const& pair) {
            return activeMapPoints.count(pair.second);
        }
    );

    // loopClosureEdges = mapDB.loopClosureEdges; // TODO: Required?
    // visualizedLoopClosureCandidates = mapDB.visualizedLoopClosureCandidates; // TODO: Required?

    prevPose = mapDB.prevPose;
    prevInputPose = mapDB.prevInputPose;
    prevSmoothPose = mapDB.prevSmoothPose;
    prevUncertainty = mapDB.prevUncertainty;

    nextMp = mapDB.nextMp;
    prevPoseKfId = mapDB.prevPoseKfId;
    lastKfCandidateId = mapDB.lastKfCandidateId;
    lastKfId = mapDB.lastKfId;
}

std::map<MpId, MapPoint>::iterator
MapDB::removeMapPoint(const MapPoint &mapPoint) {
    for (const auto &it : mapPoint.observations) {
        keyframes.at(it.first)->eraseObservation(mapPoint.id);
    }

    if (mapPoint.trackId.v != -1) {
        assert(trackIdToMapPoint.at(mapPoint.trackId) == mapPoint.id);
        trackIdToMapPoint.erase(mapPoint.trackId);
    }

    // in the rare case the keyframe would be empty of observations, do nothing
    return mapPoints.erase(mapPoints.find(mapPoint.id));
}

MpId MapDB::nextMpId() {
    nextMp++;
    return MpId(nextMp - 1);
}

std::pair<KfId, MpId> MapDB::maxIds() const {
    KfId maxKfId(-1);
    MpId maxMpId(-1);
    for (const auto &it : keyframes) {
        if (it.first > maxKfId) maxKfId = it.first;
    }
    for (const auto &it : mapPoints) {
        if (it.first > maxMpId) maxMpId = it.first;
    }
    return { maxKfId, maxMpId };
}

void MapDB::mergeMapPoints(MpId mpId1, MpId mpId2) {
    assert(mpId1 != mpId2);
    const MpId first = mpId1 < mpId2 ? mpId1 : mpId2;
    const MpId last  = mpId1 < mpId2 ? mpId2 : mpId1;
    const auto &firstMpIt = mapPoints.find(first);
    const auto &lastMpIt = mapPoints.find(last);
    assert(firstMpIt != mapPoints.end());
    assert(lastMpIt != mapPoints.end());
    TrackId lastTrackId = lastMpIt->second.trackId;
    for (auto &it : keyframes) {
        Keyframe &keyframe = *it.second;
        assert(keyframe.mapPoints.size() == keyframe.shared->keyPoints.size());
        for (size_t i = 0; i < keyframe.mapPoints.size(); ++i) {
            KpId kpId(i);
            if (keyframe.mapPoints[i] == last) {
                keyframe.mapPoints[i] = first;
                if (keyframe.keyPointToTrackId.count(kpId)) {
                    keyframe.keyPointToTrackId.at(kpId) = firstMpIt->second.trackId;
                }
                firstMpIt->second.observations.at(keyframe.id) = kpId;
                break;
            }
        }
    }

    if (lastTrackId.v != -1) {
        trackIdToMapPoint.erase(trackIdToMapPoint.find(lastTrackId));
    }
    mapPoints.erase(lastMpIt);
}

Eigen::Matrix4d MapDB::poseDifference(KfId kfId1, KfId kfId2) const {
    assert(kfId1 <= kfId2);
    const Keyframe &kf1 = *keyframes.at(kfId1);
    const Keyframe &kf2 = *keyframes.at(kfId2);
    return kf1.smoothPoseCW * kf2.smoothPoseCW.inverse();
}

void MapDB::updatePrevPose(const Keyframe &currentKeyframe, const Eigen::Matrix4d &inputPose)
{
    // log_debug("storing keyframe %d as prev pose", currentKeyframe.id.v);
    prevPoseKfId = currentKeyframe.id;
    prevInputPose = inputPose;
    prevSmoothPose = currentKeyframe.smoothPoseCW;
    prevPose = currentKeyframe.poseCW;
    prevUncertainty = currentKeyframe.uncertainty;

    const auto *prevKf = latestKeyframe();
    assert(prevKf);
}

const MapDB& getMapWithId(MapId mapId, const MapDB &mapDB, const Atlas &atlas) {
    if (mapId == CURRENT_MAP_ID) return mapDB;
    return atlas[mapId.v];
}

} // namespace slam
