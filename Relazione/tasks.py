import shlex
import subprocess
import sys
import typing as t
from functools import partial
from json import dump
from os import environ
from pathlib import Path

from argh import aliases, dispatch_commands
from slugify import slugify
from termcolor import colored as c
from termcolor import cprint

cb = partial(c, attrs=["bold"])

LATEXMK_FLAGS = (
    "-shell-escape -pdf -halt-on-error -interaction=nonstopmode -lualatex".split(" ")
)
INKSCAPE = r"C:\Program Files\Inkscape\bin\inkscape.exe"
TEX_FILE = "pii.tex"

BOOLEAN_TAGS = {
    1: (
        "Riabilitzione Remota Valvola Acqua",
        "Riabilitazione Remota Valvola Gas",
        "Chiave Remota Allarme",
    ),
    5: (
        "Intrusione Accertata",
        "Fumo Accertato",
        "Finestra 1 Chiusa",
        "Finestra 2 Chiusa",
        "Finestra 3 Chiusa",
        "Porta D'Ingresso Chiusa",
        "Chiave Fisica Allarme",
        "Fumo in cucina",
        "Fumo in bagno 1",
        "Fumo in bagno 2",
        "Gas in Cucina",
        "Allagamento in bagno",
        "Allagamento in cucina",
        "Chiusura valvola gas",
        "Chiusura valvola acqua",
    ),
}


def run(cmd: t.Sequence[str | Path], /, **kwargs) -> subprocess.CompletedProcess:
    check = kwargs.pop("check", True)
    print(
        cb("$", "green"),
        shlex.join(map(str, cmd)),
        c(f"(w/ options {kwargs})", "green") if kwargs else "",
    )
    proc = subprocess.run(cmd, **kwargs)
    if check and proc.returncode != 0:
        print(cb("Failed.", "red"))
        sys.exit(proc.returncode)
    return proc


@aliases("b")
def build(texfile: str = "") -> None:
    Path("figures").mkdir(exist_ok=True)
    cmd = ["latexmk", *LATEXMK_FLAGS]
    if texfile:
        cmd.append(texfile)
    run(cmd)


@aliases("w")
def watch(texfile: str = "") -> None:
    cmd = ["latexmk", *LATEXMK_FLAGS, "-pvc"]
    if texfile:
        cmd.append(texfile)
    run(cmd)


def show(texfile: str = "") -> None:
    cmd = ["latexmk", *LATEXMK_FLAGS, "-pv"]
    if texfile:
        cmd.append(texfile)
    run(cmd)


def svg2pdf(image_path: str) -> None:
    run(
        (
            INKSCAPE,
            "-D",
            image_path,
            "-o",
            Path("assets", Path(image_path).with_suffix(".pdf").name),
            "--export-latex",
        )
    )


def setup_environ() -> None:
    assert "TEXINPUTS" not in environ
    environ["TEXINPUTS"] = ";".join(
        (
            ".",
            str(Path(__file__).parent.resolve()),
            "",
        )
    )


def write_tags() -> None:
    tags = [
    ]

    for var, bits in BOOLEAN_TAGS.items():
        assert len(bits) <= 16
        for idx, name in enumerate(bits):
            tags.append(
                {
                    "valueSource": "opc",
                    "opcItemPath": f"ns\u003d1;s\u003d[Zelio]MW{var}.{idx}",
                    "dataType": "Boolean",
                    "name": slugify(name),
                    "tagType": "AtomicTag",
                    "opcServer": "Ignition OPC UA Server",
                },
            )

    with open("tags.json", "w") as f:
        dump({"tags": tags}, f)


def main() -> None:
    setup_environ()
    try:
        dispatch_commands(
            (
                build,
                watch,
                show,
                svg2pdf,
                write_tags,
            )
        )
    except KeyboardInterrupt:
        cprint("Interrupted, bye!", "red")


if __name__ == "__main__":
    main()
