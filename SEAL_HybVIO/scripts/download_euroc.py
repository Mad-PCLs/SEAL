import argparse

from utils.EurocDatasetManager import EurocDatasetManager

DATA_DIR = "../data/"

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-type",
        help="Specify the type of data to prepare: 'vins' or 'hybvio'",
        required=True
    )
    args = parser.parse_args()

    manager = EurocDatasetManager(
        dataset_type=args.type,
        data_dir=DATA_DIR
    )
    manager.check_and_prepare_data()
    print("Data preparation complete.")
