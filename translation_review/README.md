# Translation review export

One file per language: `<code>.md`. Each file starts with the review instructions, then all 232 texts of the VR menu
and keyboard (number, where it is shown and how much room it has, English, translation). Paste one whole file into
another AI model (or give it to a native speaker) and send the answer back: the numbers let the corrections be applied
to `lang/<code>.po`.

`all_languages.csv` = every text of every language in one table (Excel / Google Sheets).

Made by `python scripts/lang_tool.py review translation_review`.
