import os
import sys
import shutil

import subprocess

import json
from datetime import datetime

import time

class VINSMonoExecutor(object):
    def __init__(
        self,
        output_root_folder,
        rosbag_path,
        vins_launch_file,
        seal_params_path,
        vins_package,
        default_output_folder,
        setup_script_path="../devel/setup.bash",
        wait_for_ros_master_s=5
    ):
        """
        Initialize the VINS-Mono executor for automated experiments.

        Args:
            output_root_folder (str): Path to the root folder where output will be stored.
            rosbag_path (str): Path to the rosbag file to play.
            vins_launch_file (str): Name of the VINS launch file.
            seal_params_path (str): Path to the SEAL parameters file. 
            vins_package (str): Name of the VINS ROS package.
            default_output_folder (str): Default output folder created by VINS.
            setup_script_path (str): Path to the setup.bash script to source before starting ROS.
            wait_for_ros_master_s (int): How many seconds to wait for ROS Master Node
                to initialize, before feeding the rosbag file. Defaults to 5 seconds.
        """
        self.output_root_folder = output_root_folder
        self.rosbag_path = rosbag_path
        self.vins_launch_file = vins_launch_file
        self.seal_params_path = seal_params_path
        self.vins_package = vins_package
        self.default_output_folder = default_output_folder
        self.setup_script_path = setup_script_path
        self.wait_for_ros_master_s = wait_for_ros_master_s

        # Process handles
        self.rosmaster_process = None  # type: subprocess.Popen
        self.rosbag_process = None  # type: subprocess.Popen

        # Derived paths
        self.stdout_path = os.path.join(self.output_root_folder, "vins_stdout.txt")
        self.execution_info_path = os.path.join(self.output_root_folder, "execution_info.json")

    def _source_setup_script(self):
        """
        Source the ROS setup.bash script to initialize the environment.
        
        Raises:
            FileNotFoundError: If the setup script doesn't exist.
            subprocess.CalledProcessError: If sourcing fails.
        """
        print("Sourcing " + self.setup_script_path + "...", end='')
        if not os.path.exists(self.setup_script_path):
            raise FileNotFoundError(
                "ROS setup script not found at {script_path}".format(script_path = self.setup_script_path)
            )

        # Using bash -c to properly source the script and preserve the environment
        command = [
            "bash",
            "-c",
            "source {script_path} && env".format(script_path = self.setup_script_path)
        ]

        # Execute the command and capture the environment
        proc = subprocess.Popen(
            command,
            stdout=subprocess.PIPE,
            universal_newlines=True
        )
        
        # Parse the environment variables and update os.environ
        for line in proc.stdout:
            if '=' in line:
                key, value = line.strip().split('=', 1)
                os.environ[key] = value
        
        proc.wait()
        if proc.returncode != 0:
            raise subprocess.CalledProcessError(
                proc.returncode,
                command
            )
        print("[Done]")

    def _prepare_output_folder(self):
        """
        Prepare the output folder by:
        1. Creating it if it doesn't exist
        2. Cleaning it if it exists
        3. Copying the SEAL params file
        4. Creating execution info file
        """
        # Clean or create output folder
        if os.path.exists(self.output_root_folder):
            shutil.rmtree(self.output_root_folder)
        os.makedirs(self.output_root_folder)

        # Copy SEAL params file
        if os.path.exists(self.seal_params_path):
            shutil.copy(
                self.seal_params_path,
                os.path.join(
                    self.output_root_folder,
                    os.path.basename(self.seal_params_path)
                )
            )
        else:
            raise FileNotFoundError(
                "SEAL params file not found at {script_path}".format(script_path = self.setup_script_path)
            )

        # Create execution info
        execution_info = {
            "timestamp": datetime.now().isoformat(),
            "rosbag_path": self.rosbag_path,
            "vins_launch_file": self.vins_launch_file,
            "seal_params_path": self.seal_params_path,
            "output_folder": self.output_root_folder,
            "setup_script_path": self.setup_script_path
        }

        with open(self.execution_info_path, "w") as f:
            json.dump(execution_info, f, indent=4)

    def _cleanup(self):
        """Clean up processes the might be running and folders we created so far."""
        # Stop the processes that might be running
        if self.rosbag_process:
            self.stop_process(self.rosbag_process)
        if self.rosmaster_process:
            self.stop_process(self.rosmaster_process)

        # Delete any folders/files
        if os.path.exists(self.default_output_folder):
            shutil.rmtree(self.default_output_folder)
        if os.path.exists(self.output_root_folder):
            shutil.rmtree(self.output_root_folder)

        # Flush the buffers
        sys.stdout.flush()
        sys.stderr.flush()
        sys.stdin.flush()

    def start_ros_master(self):
        """
        Start the ROS master and VINS-Mono node.
        
        Raises:
            subprocess.CalledProcessError: If the process fails to start.
        """
        print("Starting ROS Master node...", end='')
        try:
            self._prepare_output_folder()
            
            # Start roscore and VINS
            command = [
                "roslaunch",
                self.vins_package,
                self.vins_launch_file
            ]

            with open(self.stdout_path, "w") as stdout_file:
                self.rosmaster_process = subprocess.Popen(
                    command,
                    stdout=stdout_file,
                    stderr=subprocess.STDOUT
                )

        except Exception as e:
            self._cleanup()
            raise

        print("[Done]")
        
    def feed_rosbag(self):
        """
        Play the rosbag and wait for completion.
        
        Raises:
            subprocess.CalledProcessError: If the process fails.
            FileNotFoundError: If rosbag file doesn't exist.
        """
        print("Feeding rosbag data: " + self.rosbag_path + "...", end='')
        sys.stdout.flush()
        if not os.path.exists(self.rosbag_path):
            raise FileNotFoundError(
                "Rosbag file not found at {script_path}".format(script_path = self.setup_script_path)
            )

        try:
            command = [
                "rosbag",
                "play",
                self.rosbag_path
            ]

            with open(self.stdout_path, "a") as stdout_file:
                self.rosbag_process = subprocess.Popen(
                    command,
                    stdout=stdout_file,
                    stderr=subprocess.STDOUT
                )

            # Wait for completion
            self.rosbag_process.wait()

            if self.rosbag_process.returncode != 0:
                raise subprocess.CalledProcessError(
                    self.rosbag_process.returncode, 
                    command
                )

            # Move VINS output to our output folder
            if os.path.exists(self.default_output_folder):
                shutil.move(
                    self.default_output_folder,
                    os.path.join(self.output_root_folder, "vins_output")
                )

        except Exception as e:
            self._cleanup()
            raise

        print("[Done]")

    def stop_process(self, proc = None):
        """
        Gracefully stop the provided process. Used for early exit, during _cleanup(),
        and to stop the ROS Master Node, once the experiment is complete.

        Args:
            proc (subprocess.Popen, optional): The process to stop. Defaults to None.
        """
        if proc:
            print("Gracefully stopping process: " + str(os.getpgid(proc.pid)) + " ...", end='')
            proc.kill()
            proc = None

            print("[Done]")

    def execute(self):
        """
        Execute the complete VINS-Mono experiment.
        
        Raises:
            subprocess.CalledProcessError: If any subprocess fails.
        """
        try:
            self._source_setup_script()
            self.start_ros_master()

            print(
                "Waiting ROS master to start for " \
                    + str(self.wait_for_ros_master_s) \
                    + " seconds...",
                end=''
            )
            sys.stdout.flush()
            time.sleep(self.wait_for_ros_master_s)
            print("[Done]")

            self.feed_rosbag()
            
        except (KeyboardInterrupt, Exception) as e:
            self._cleanup()
            raise
            
        finally:
            if self.rosmaster_process: 
                self.stop_process(self.rosmaster_process)
