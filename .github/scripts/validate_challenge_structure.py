import os
import sys
import yaml
import subprocess

REQUIRED_FILES = ['flag.txt', 'challenge.yaml']
CHALLENGES_DIR = 'challenges'

def get_changed_files():
    # Only get added or modified files
    result = subprocess.run(
        ['git', 'diff', '--name-only', '--diff-filter=AM', 'origin/main'],
        stdout=subprocess.PIPE,
        check=True
    )
    return result.stdout.decode().splitlines()

def find_new_challenge_dirs(changed_files):
    challenge_dirs = set()
    for file in changed_files:
        parts = file.split('/')
        if len(parts) >= 3 and parts[0] == CHALLENGES_DIR:
            challenge_dir = os.path.join(parts[0], parts[1], parts[2])
            challenge_dirs.add(challenge_dir)
    return list(challenge_dirs)

def validate_challenge_folder(folder):
    print(f"Validating {folder}...")
    if not os.path.isdir(folder):
        print(f"ERROR: {folder} is not a directory!")
        return False

    files = os.listdir(folder)
    missing = [f for f in REQUIRED_FILES if f not in files]
    if missing:
        print(f"ERROR: {folder} is missing required files: {', '.join(missing)}")
        return False

    # Validate challenge.yaml
    yaml_path = os.path.join(folder, 'challenge.yaml')
    with open(yaml_path, 'r') as f:
        try:
            data = yaml.safe_load(f)
        except yaml.YAMLError as e:
            print(f"ERROR: {yaml_path} is not valid YAML: {e}")
            return False
        if data is None:
            print(f"ERROR: {yaml_path} is not valid YAML: {e}")
            return False

    # Optional: Check specific fields in YAML
    required_fields = ['name', 'author']
    for field in required_fields:
        if field not in data:
            print(f"ERROR: {yaml_path} is missing required field '{field}'")
            return False

    return True

def main():
    changed_files = get_changed_files()
    challenge_dirs = find_new_challenge_dirs(changed_files)

    if not challenge_dirs:
        print("No new challenge folders detected.")
        sys.exit(1)

    all_valid = True
    for folder in challenge_dirs:
        if not validate_challenge_folder(folder):
            all_valid = False

    if not all_valid:
        print("❌ Validation failed.")
        sys.exit(1)

    print("✅ All new challenges validated successfully!")

if __name__ == '__main__':
    main()
