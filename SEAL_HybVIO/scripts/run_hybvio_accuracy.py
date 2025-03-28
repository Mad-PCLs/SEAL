import argparse

from accuracy.SequenceRunner import SequenceRunner

BENCHMARK_FOLDER = "../data/benchmark/"
RAW_DATA_FOLDER = "../data/raw/"
OUTPUT_ROOT_FOLDER = "../output_accuracy/"
BINARY_PATH = "../HybVIO/build/main"
ORB_VOCAB_PATH = "../HybVIO/data/orb_vocab.dbow2"
SEAL_PARAMS_PATH = "../HybVIO/SEAL/seal_params.yaml"

SEQUENCE_NAMES = [
    "euroc-mh-01-easy",
    "euroc-mh-02-easy",
    "euroc-mh-03-medium",
    "euroc-mh-04-difficult",
    "euroc-mh-05-difficult",
    "euroc-v1-01-easy",
    "euroc-v1-02-medium",
    "euroc-v1-03-difficult",
    "euroc-v2-01-easy",
    "euroc-v2-02-medium",
    "euroc-v2-03-difficult"
]

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Run HybVIO on sequences and calculate metrics."
    )
    parser.add_argument(
        "--sequence",
        type=str,
        default=None,
        help="Name of the sequence to run. If not provided, run all sequences.",
    )
    parser.add_argument(
        "--rerun",
        action="store_true",
        help="Delete existing output folders and rerun the sequences.",
    )
    args = parser.parse_args()

    runner = SequenceRunner(
        benchmarks_folder=BENCHMARK_FOLDER,
        raw_data_folder=RAW_DATA_FOLDER,
        output_root_folder=OUTPUT_ROOT_FOLDER,
        binary_path=BINARY_PATH,
        orb_vocab_path=ORB_VOCAB_PATH,
        seal_params_path=SEAL_PARAMS_PATH,
        metric="RMSE"
    )

    sequence_names = [args.sequence] if args.sequence else None
    runner.run_sequences(sequence_names=sequence_names, rerun=args.rerun)
