FRENCH VERSION  
==============

# Key Detector — plugin VST3 de détection de tonalité (FL Studio)

Plugin transparent (l'audio passe sans modification) qui affiche en temps réel la tonalité du morceau
(ex. *Eb major*, *Mib majeur*, *Camelot 5B*), un indice de correspondance, la tonalité relative,
la 2e hypothèse et les 12 notes détectées. Interface redimensionnable.

## Obtenir l'installeur Windows (une seule fois, ~5 minutes)

Windows demande un compilateur particulier (Visual Studio) : il tourne gratuitement chez GitHub.

1. Crée un compte sur github.com puis un dépôt vide (bouton **New**, nom : `KeyDetector`).
2. Dans le dépôt : **Add file > Upload files**, glisse **tout le contenu de ce dossier**
   (y compris le dossier caché `.github`), puis **Commit changes**.
3. Onglet **Actions** : la compilation démarre seule (≈ 10–15 min). Attends la coche verte.
4. Onglet **Code** > à droite **Releases** > `Key Detector - dernière version` > télécharge **KeyDetector-Setup.exe**.

## Installer dans FL Studio

1. Lance `KeyDetector-Setup.exe` (si Windows affiche « SmartScreen » : *Informations complémentaires > Exécuter quand même* — l'installeur n'est pas signé).
2. FL Studio : **Options > Manage plugins > Find plugins**.
3. Ajoute **Key Detector** sur une piste du Mixer, lance la lecture.

Sans installeur : décompresse `KeyDetector-VST3-Windows.zip` dans `C:\Program Files\Common Files\VST3`.

## Utilisation

- Place-le sur le morceau complet (ou le master) ; le résultat se stabilise après quelques secondes.
- **MEMORY** : durée sur laquelle l'analyse se souvient (30 s par défaut). Courte = suit les modulations, longue = plus stable.
- **RESET** : repart de zéro (changement de morceau).
- **MATCH %** : ressemblance avec le profil de la tonalité. **ALT** montre l'hypothèse suivante : si les deux sont proches
  (ex. tonalité et sa relative), c'est ambigu, ce qui est normal en musique.

## Comment ça marche

FFT 16384 points (recouvrement 50 %) → énergie 65 Hz–4,2 kHz regroupée en 12 classes de hauteur → accumulation
avec oubli exponentiel → corrélation avec les profils de Krumhansl-Kessler (12 majeures + 12 mineures).

## Compiler soi-même (Windows)

```
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```
JUCE est téléchargé automatiquement. Le plugin est dans `build/KeyDetector_artefacts/Release/VST3`.
Sur macOS/Linux, `cmake -B build && cmake --build build` produit VST3 et Standalone (AU sur macOS).

## Licence

JUCE est sous licence AGPLv3 / commerciale : pour distribuer le plugin, publie le code source ou prends une licence JUCE.

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ENGLISH VERSION  
===============


Key Detector — VST3 Pitch Detection Plugin (FL Studio)

A transparent plugin (audio passes through without modification) that displays the key of a track in real time (e.g., Eb major, E-flat major, Camelot 5B), along with a match confidence indicator, the relative key, the second-best guess, and all 12 detected pitch classes. The interface is resizable.

Get the Windows Installer (one-time setup, ~5 minutes)

Windows requires a specific compiler (Visual Studio), which runs for free on GitHub.

Create an account on github.com and create an empty repository (click New, name it KeyDetector).

In the repository, go to Add file > Upload files and drag in the entire contents of this folder (including the hidden .github folder), then click Commit changes.

Open the Actions tab. The build will start automatically (approximately 10–15 minutes). Wait until the green check mark appears.

Go to the Code tab, find Releases on the right, open Key Detector - latest version, and download KeyDetector-Setup.exe.

Install in FL Studio

Run KeyDetector-Setup.exe (if Windows displays a SmartScreen warning, click More info > Run anyway — the installer is unsigned).

In FL Studio, go to Options > Manage plugins > Find plugins.

Add Key Detector to a Mixer track and start playback.

Without the installer: Extract KeyDetector-VST3-Windows.zip into C:\Program Files\Common Files\VST3.

How to Use

Insert the plugin on the full track or master channel. The results will stabilize after a few seconds.

MEMORY: Sets how long the analysis remembers previous audio (30 seconds by default). Shorter = follows key changes more closely; longer = more stable results.

RESET: Starts the analysis from scratch (use when switching tracks).

MATCH %: Indicates how closely the detected notes match the key profile. ALT displays the next-best guess. If both results are close (e.g., a key and its relative key), the result is ambiguous, which is normal in music.

How It Works

16384-point FFT (50% overlap) → energy between 65 Hz and 4.2 kHz grouped into 12 pitch classes → accumulation with exponential forgetting → correlation against Krumhansl-Kessler key profiles (12 major and 12 minor keys).

Build It Yourself (Windows)

cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release

JUCE is downloaded automatically. The plugin will be located in build/KeyDetector_artefacts/Release/VST3.

On macOS/Linux, cmake -B build && cmake --build build produces VST3 and Standalone builds (AU on macOS).





