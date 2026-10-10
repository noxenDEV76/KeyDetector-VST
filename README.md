# 🎹 KEY DETECTOR — VST3 KEY DETECTION PLUGIN

🎧 **Detect the key of any song directly inside FL Studio!**

A free, real-time key detection plugin designed for music producers. 🔥

## 🇫🇷 VERSION FRANÇAISE

### 🎼 Qu'est-ce que Key Detector ?

**Key Detector** est un plugin VST3 qui analyse l'audio en temps réel pour détecter la tonalité musicale de ton morceau, sans modifier le son original. 🎧

✨ **Fonctionnalités principales :**
- 🎹 Détection de tonalité en temps réel (Eb Major, F# Minor, etc.).
- 🎯 Indicateur de correspondance avec la tonalité détectée.
- 🔄 Affichage de la tonalité relative.
- 🥈 Deuxième hypothèse de tonalité avec ALT.
- 🎼 Affichage des 12 notes détectées.
- 🎡 Compatibilité avec le système Camelot (DJ).
- 📏 Interface redimensionnable.
- 🔊 Audio totalement transparent : aucun effet ajouté au son.

### 🚀 Comment installer le plugin sur Windows ?

**Étape 1 — 📦 Préparer GitHub**
1. Crée un compte sur [GitHub](https://github.com/).
2. Crée un nouveau dépôt nommé `KeyDetector`.
3. Envoie tous les fichiers du projet, y compris le dossier caché `.github`.
4. Clique sur **Commit changes**.

**Étape 2 — ⚙️ Compiler le plugin**
1. Ouvre l'onglet **Actions** de ton dépôt.
2. Attends que la compilation automatique se termine.
3. Vérifie que le workflow affiche une coche verte ✅.

**Étape 3 — ⬇️ Télécharger l'installeur**
1. Retourne dans l'onglet **Code**.
2. Ouvre la section **Releases**.
3. Sélectionne la dernière version de Key Detector.
4. Télécharge `KeyDetector-Setup.exe`.

⏳ La compilation peut prendre environ 10 à 15 minutes.

### 🎛️ Comment l'utiliser dans FL Studio ?

1. Lance `KeyDetector-Setup.exe`.
2. Ouvre FL Studio.
3. Va dans **Options → Manage plugins → Find plugins**.
4. Ajoute **Key Detector** sur une piste du Mixer.
5. Lance ton morceau et observe la tonalité détectée. 🎶

💡 **Astuce :** place le plugin sur le Master pour analyser le morceau complet.

### 🧠 Les commandes du plugin

- 🕒 **MEMORY** — règle la durée de mémoire de l'analyse. Une valeur courte suit mieux les changements de tonalité ; une valeur longue offre davantage de stabilité.
- 🔄 **RESET** — réinitialise l'analyse lorsque tu changes de morceau.
- 🎯 **MATCH %** — indique la correspondance entre les notes analysées et le profil de tonalité.
- 🥈 **ALT** — affiche la deuxième tonalité la plus probable. Si les deux résultats sont proches, la tonalité peut être ambiguë.

### 🔬 Comment fonctionne l'analyse ?

Key Detector utilise plusieurs étapes de traitement du signal :

1. 📊 Analyse fréquentielle par FFT de 16 384 points, avec 50 % de recouvrement.
2. 🎵 Extraction de l'énergie entre 65 Hz et 4,2 kHz.
3. 🎹 Regroupement des fréquences en 12 classes de notes.
4. 🧠 Accumulation des informations avec une mémoire à oubli exponentiel.
5. 🎼 Comparaison avec les profils de tonalité de Krumhansl-Kessler : 12 tonalités majeures et 12 mineures.

### 🛠️ Compiler le plugin soi-même

Prérequis : Git, CMake et Visual Studio 2022 avec les outils C++.

```bash
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

📂 Le plugin VST3 compilé se trouve normalement dans :

`build/KeyDetector_artefacts/Release/VST3`

JUCE est téléchargé automatiquement par le projet si sa configuration le prévoit.

### 📜 Licence

JUCE est disponible sous licence AGPLv3 ou commerciale, selon les conditions applicables. Si tu distribues le plugin, vérifie les obligations de licence de JUCE et de toutes les dépendances utilisées.

---

## 🇬🇧 ENGLISH VERSION

### 🎹 What is Key Detector?

**Key Detector** is a free VST3 plugin that analyzes audio in real time and detects the musical key of your track without modifying the original sound. 🎧

✨ **Key features:**
- 🎹 Real-time musical key detection.
- 🎯 Key match indicator.
- 🔄 Relative key detection.
- 🥈 Second-best key guess with ALT.
- 🎼 All 12 detected pitch classes displayed.
- 🎡 Camelot key notation for DJs.
- 📏 Resizable interface.
- 🔊 Transparent audio pass-through — no added effects.

### 🚀 Get the Windows Installer

**1️⃣ Set up GitHub**
- Create an account at [GitHub](https://github.com/).
- Create a new repository named `KeyDetector`.
- Upload the entire project, including the hidden `.github` folder.
- Click **Commit changes**.

**2️⃣ Build the plugin**
- Open the repository's **Actions** tab.
- Wait for the automated build to finish.
- Make sure the workflow displays a green check mark ✅.

**3️⃣ Download the installer**
- Return to the **Code** tab.
- Open **Releases**.
- Select the latest Key Detector release.
- Download `KeyDetector-Setup.exe`.

⏳ The automated build may take approximately 10–15 minutes.

### 🎛️ Install in FL Studio

1. Run `KeyDetector-Setup.exe`.
2. Open FL Studio.
3. Navigate to **Options → Manage plugins → Find plugins**.
4. Load **Key Detector** onto a Mixer track.
5. Start playback and watch the detected key update in real time. 🎶

💡 **Pro tip:** Insert it on the Master channel to analyze the entire track.

### 🧠 Plugin controls

- 🕒 **MEMORY** — controls how long previous analysis contributes to the result. Shorter memory responds faster to key changes; longer memory provides more stability.
- 🔄 **RESET** — clears the current analysis when switching tracks.
- 🎯 **MATCH %** — indicates how closely the detected notes match the selected key profile.
- 🥈 **ALT** — displays the second-most-likely key. Similar scores can indicate an ambiguous result.

### 🔬 How does it work?

The plugin uses a signal-processing pipeline:

1. 📊 16,384-point FFT with 50% overlap.
2. 🎵 Frequency-energy analysis between 65 Hz and 4.2 kHz.
3. 🎹 Mapping into 12 pitch classes.
4. 🧠 Exponential-decay accumulation over time.
5. 🎼 Correlation against Krumhansl-Kessler profiles for all 12 major and 12 minor keys.

### 🛠️ Build it yourself

Requirements: Git, CMake and Visual Studio 2022 with the C++ development tools.

```bash
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

📂 The compiled VST3 should be located at:

`build/KeyDetector_artefacts/Release/VST3`

JUCE is downloaded automatically if configured in the project.

### 📜 License

JUCE is available under the AGPLv3 or a commercial license, subject to the applicable terms. Review the licensing requirements before distributing the plugin.

🎧 **Made for producers, by a producer. Find your key. Create faster.** 🎹🔥
