import os
import sys
import yaml
import subprocess

REQUIRED_FILES = ['flag.txt', 'challenge.yaml','solution/README.md','README.md']
REQUIRED_DIRS = ['solution','dist','src']
CHALLENGES_DIR = 'challenges'

def get_changed_files():
    # Only get added or modified files
    result = subprocess.run(
        ['git', 'diff', '--name-only', '--diff-filter=AM', 'origin/main...HEAD'],
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

def list_files_one_level(root_folder):
    result = []
    for dirpath, dirnames, filenames in os.walk(root_folder):
        depth = dirpath[len(root_folder):].count(os.sep)
        if depth > 1:
            continue
        for name in filenames:
            full_path = os.path.relpath(os.path.join(dirpath, name), root_folder)
            result.append(full_path)
    return result

def validate_challenge_yaml(data):
    required_fields = ['name',
                       'description',
                       'author',
                      'type']

    required_for_type = {
        'http': ["link"],
        'nc': ['host','port'],
        'static':[]
    }
    
    for field in required_fields:
        if field not in data:
            print(f"ERROR: {yaml_path} is missing required field '{field}'")
            return False

    challenge_type = data.get("type","")

    if challenge_type == "" or challenge_type not in required_for_type:
        print(f"ERROR: {yaml_path} has invalid challenge type '{challenge_type}'")
        return False
        
    for field in required_for_type[challenge_type]:
        if field not in data:
            print(f"ERROR: {yaml_path} is missing required field '{field}' for type '{challenge_type}'")
            return False
    
    return True

def validate_challenge_folder(folder):
    print(f"Validating {folder}...")
    if not os.path.isdir(folder):
        print(f"ERROR: {folder} is not a directory!")
        return False

    files = list_files_one_level(folder)
    
    print(files)
    
    missing = [f for f in REQUIRED_FILES if f not in files]
    if missing:
        print(f"ERROR: {folder} is missing required files: {', '.join(missing)}")
        return False

    missing = [f for f in REQUIRED_DIRS if f not in files or not os.path.isdir(f)]
    if missing:
        print(f"ERROR: {folder} is missing required directories: {', '.join(missing)}")
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
            print(f"ERROR: {yaml_path} is not valid YAML")
            return False

    return validate_challenge_yaml(data)

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
