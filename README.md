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
