#!/usr/bin/env python3
"""Utility for cloning the CEF source tree matching the configured binary build."""

from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path

CEF_VERSION = "140.1.14+geb1c06e+chromium-140.0.7339.185"
CEF_REPOSITORY = "https://bitbucket.org/chromiumembedded/cef.git"


def run_command(command: list[str], *, cwd: Path | None = None) -> None:
    """Execute *command* and raise a helpful error message on failure."""
    try:
        subprocess.run(command, cwd=cwd, check=True)
    except subprocess.CalledProcessError as exc:  # pragma: no cover - defensive.
        cmd_str = " ".join(command)
        location = f" in {cwd}" if cwd else ""
        raise SystemExit(
            f"Command '{cmd_str}'{location} failed with exit code {exc.returncode}."
        ) from exc


def ensure_repo(version: str, repo_url: str, destination: Path) -> None:
    """Clone or update the CEF repository so it matches *version* at *destination*."""
    tag = f"cef_{version}"

    if destination.exists():
        git_dir = destination / ".git"
        if not git_dir.is_dir():
            raise SystemExit(
                f"Destination '{destination}' exists but is not a Git repository. "
                "Please remove it or choose a different --output-dir."
            )

        run_command(["git", "fetch", "--tags", repo_url], cwd=destination)
        run_command(["git", "fetch", "origin", tag], cwd=destination)
        run_command(["git", "checkout", "--force", tag], cwd=destination)
        run_command(["git", "clean", "-fdx"], cwd=destination)
    else:
        destination.parent.mkdir(parents=True, exist_ok=True)
        run_command(
            [
                "git",
                "clone",
                "--branch",
                tag,
                "--depth",
                "1",
                repo_url,
                str(destination),
            ]
        )


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(
        description=(
            "Clone the Chromium Embedded Framework (CEF) source tree for reference "
            "when working on the webbrowser target."
        )
    )
    parser.add_argument(
        "--version",
        default=CEF_VERSION,
        help=(
            "CEF version tag to check out (default: %(default)s). The script will "
            "look for a tag named 'cef_<version>'."
        ),
    )
    parser.add_argument(
        "--repo",
        default=CEF_REPOSITORY,
        help="CEF git repository URL to clone (default: %(default)s).",
    )

    default_destination = (
        Path(__file__).resolve().parents[1] / "third_party" / "cef_source"
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=default_destination,
        help=(
            "Directory to clone the repository into. The directory will be "
            "created if it does not exist (default: %(default)s)."
        ),
    )

    args = parser.parse_args(argv)

    ensure_repo(args.version, args.repo, args.output_dir)
    print(
        f"CEF source for version {args.version} is available at {args.output_dir}.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
