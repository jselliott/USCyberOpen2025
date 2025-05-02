import os
from pathlib import Path

README_PATH = Path("README.md")
CHALLENGES_DIR = Path("challenges")
TOC_START = "<!-- BEGIN TOC -->"
TOC_END = "<!-- END TOC -->"

def get_challenges():
    rows = []
    for category_dir in CHALLENGES_DIR.iterdir():
        if category_dir.is_dir():
            for challenge_dir in category_dir.iterdir():
                if challenge_dir.is_dir():
                    meta = {}
                    yaml_path = challenge_dir / "challenge.yaml"
                    if yaml_path.exists():
                        with open(yaml_path) as f:
                            for line in f:
                                if ":" in line:
                                    key, val = line.split(":", 1)
                                    meta[key.strip()] = val.strip()
                    rows.append((
                        category_dir.name,
                        challenge_dir.name,
                        meta.get("name", challenge_dir.name)
                    ))
    return sorted(rows)

def generate_table(challenges):
    lines = [
        "| Category | Challenge | Description |",
        "|----------|-----------|-------------|"
    ]
    for cat, slug, desc in challenges:
        lines.append(f"| {cat} | [{slug}](challenges/{cat}/{slug}) | {desc} |")
    return "\n".join(lines)

def update_readme():
    toc = generate_table(get_challenges())
    new_toc_block = f"{TOC_START}\n{toc}\n{TOC_END}"

    if README_PATH.exists():
        contents = README_PATH.read_text()
        if TOC_START in contents and TOC_END in contents:
            # Replace the old TOC block
            start = contents.index(TOC_START)
            end = contents.index(TOC_END) + len(TOC_END)
            contents = contents[:start] + new_toc_block + contents[end:]
        else:
            # Append to end
            contents += f"\n\n## Challenge Table of Contents\n{new_toc_block}"
    else:
        contents = f"# CTF Challenges\n\n## Challenge Table of Contents\n{new_toc_block}"

    README_PATH.write_text(contents)
    print("✅ README.md updated with challenge table.")

if __name__ == "__main__":
    update_readme()
