import os
import shutil
from pathlib import Path
import subprocess
from typing import Dict, Literal, Optional

from utils.BenchmarkConverter import BenchmarkConverter

class EurocDatasetManager:
    def __init__(
        self,
        dataset_type: Literal["vins", "hybvio"],
        data_dir: str
    ):
        self.dataset_type = dataset_type
        self.link_prefix = "http://robotics.ethz.ch/~asl-datasets/ijrr_euroc_mav_dataset"
        self.raw_dir = os.path.join(data_dir, "raw/")
        self.benchmark_dir = os.path.join(data_dir, "benchmark/")
        self.datasets = [
            {"name": "euroc-mh-01-easy", "link": "machine_hall/MH_01_easy/MH_01_easy.zip"},
            {"name": "euroc-mh-02-easy", "link": "machine_hall/MH_02_easy/MH_02_easy.zip"},
            {"name": "euroc-mh-03-medium", "link": "machine_hall/MH_03_medium/MH_03_medium.zip"},
            {"name": "euroc-mh-04-difficult", "link": "machine_hall/MH_04_difficult/MH_04_difficult.zip"},
            {"name": "euroc-mh-05-difficult", "link": "machine_hall/MH_05_difficult/MH_05_difficult.zip"},
            {"name": "euroc-v1-01-easy", "link": "vicon_room1/V1_01_easy/V1_01_easy.zip"},
            {"name": "euroc-v1-02-medium", "link": "vicon_room1/V1_02_medium/V1_02_medium.zip"},
            {"name": "euroc-v1-03-difficult", "link": "vicon_room1/V1_03_difficult/V1_03_difficult.zip"},
            {"name": "euroc-v2-01-easy", "link": "vicon_room2/V2_01_easy/V2_01_easy.zip"},
            {"name": "euroc-v2-02-medium", "link": "vicon_room2/V2_02_medium/V2_02_medium.zip"},
            {"name": "euroc-v2-03-difficult", "link": "vicon_room2/V2_03_difficult/V2_03_difficult.zip"}
        ]
        self.to_seconds = 1000 * 1000 * 1000

    def check_and_prepare_data(self):
        if self.dataset_type == "vins":
            self._prepare_vins_data()
        elif self.dataset_type == "hybvio":
            self._prepare_hybvio_data()
        else:
            raise ValueError("Invalid dataset type specified. Choose 'vins' or 'hybvio'.")

    def _prepare_vins_data(
        self,
        specific_dataset: Optional[Dict[str, str]] = None
    ):
        datasets = [specific_dataset] if specific_dataset else self.datasets
        for dataset in datasets:
            dataset_path = os.path.join(
                self.raw_dir,
                dataset['name']
            )
            if not os.path.exists(dataset_path) or not os.listdir(dataset_path):
                print(f"Downloading and extracting {dataset['name']}...")
                self._download_and_extract(dataset)
            else:
                print(f"{dataset['name']} has been already downloaded and extracted.")

    def _prepare_hybvio_data(self):
        for dataset in self.datasets:
            benchmark_path = os.path.join(
                self.benchmark_dir,
                dataset['name']
            )
            if not os.path.exists(benchmark_path) or not os.listdir(benchmark_path):
                print(f"Preparing {dataset['name']} for hybvio...")
                # Ensure raw data are available
                self._prepare_vins_data(
                    specific_dataset=dataset
                )
                # Convert raw data to benchmark format, as expected by HybVIO
                curr_raw_path = os.path.join(self.raw_dir, dataset["name"])
                converter = BenchmarkConverter(
                    dataset,
                    curr_raw_path,
                    benchmark_path
                )
                # If there is an Exception or Interrupt, make
                # sure you don't leave partially converted folders
                # because next time this code is invoked it will skip
                # folders that are not empty - consider them already prepared.
                try:
                    converter.convert_to_benchmark_format()
                except (KeyboardInterrupt, Exception) as e:
                    print(f"Error during extraction: {e}. Cleaning up...")
                    if os.path.exists(benchmark_path):
                        shutil.rmtree(benchmark_path)
                    raise

            else:
                print(f"{dataset['name']} is already prepared for hybvio.")

    def _download_and_extract(self, dataset: Dict[str, str]):
        rawdir = os.path.join(self.raw_dir, dataset['name'])
        zip_path = rawdir + ".zip"

        # Download the dataset
        if not os.path.exists(zip_path):
            Path(rawdir).mkdir(parents=True, exist_ok=True)
            try:
                subprocess.run(
                    [
                        "wget", self.link_prefix + "/" + dataset['link'],
                        "-O", zip_path
                    ],
                    stdout=subprocess.DEVNULL,
                    stderr=subprocess.DEVNULL,
                    check=True  # Raise an exception if the command fails
                )
            except (KeyboardInterrupt, Exception) as e:
                # Clean up in case of an exception or an interrupt
                print(f"Error during download: {e}. Cleaning up...")
                if os.path.exists(zip_path):
                    os.remove(zip_path)
                raise

        try:
            Path(rawdir).mkdir(parents=True, exist_ok=True)
            subprocess.run(
                [
                    "unzip", zip_path,
                    "-d", rawdir
                ],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
                check=True  # Raise an exception if the command fails
            )
            os.remove(zip_path)
        except (KeyboardInterrupt, Exception) as e:
            # Clean up in case of an exception or an interrupt
            print(f"Error during extraction: {e}. Cleaning up...")
            if os.path.exists(rawdir):
                shutil.rmtree(rawdir)
            raise
