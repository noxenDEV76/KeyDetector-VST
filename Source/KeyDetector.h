#pragma once
// Moteur de détection de tonalité (sans dépendance JUCE, testable seul).
// Méthode : FFT -> chromagramme (12 classes de hauteur) accumulé avec oubli
// exponentiel -> corrélation avec les profils de Krumhansl-Kessler (24 tonalités).

#include <array>
#include <atomic>
#include <cmath>
#include <vector>
#include <algorithm>

class KeyDetector
{
public:
    static constexpr double kPi = 3.14159265358979323846;
    static constexpr int fftOrder = 14;                 // 16384 points
    static constexpr int fftSize  = 1 << fftOrder;
    static constexpr int hopSize  = fftSize / 2;        // 50 % de recouvrement

    struct Result
    {
        int   tonic       = -1;     // 0 = C ... 11 = B, -1 = pas de résultat
        bool  minor       = false;
        float correlation = 0.f;    // corrélation de Pearson du meilleur candidat
        float margin      = 0.f;    // écart avec le 2e meilleur candidat
        int   altTonic    = -1;     // 2e meilleur candidat
        bool  altMinor    = false;
        float altCorrelation = 0.f;
        std::array<float, 24> scores {};   // corrélation des 24 tonalités : index = (minor ? 12 : 0) + tonique
    };

    KeyDetector()
    {
        // fenêtre de Hann
        for (int i = 0; i < fftSize; ++i)
            window[(size_t) i] = 0.5f - 0.5f * std::cos (2.0 * kPi * i / fftSize);

        // table de twiddles
        for (int i = 0; i < fftSize / 2; ++i)
        {
            twCos[(size_t) i] =  (float) std::cos (2.0 * kPi * i / fftSize);
            twSin[(size_t) i] = -(float) std::sin (2.0 * kPi * i / fftSize);
        }

        // bit reversal
        for (int i = 0; i < fftSize; ++i)
        {
            int r = 0;
            for (int b = 0; b < fftOrder; ++b)
                if (i & (1 << b)) r |= 1 << (fftOrder - 1 - b);
            bitRev[(size_t) i] = r;
        }
        for (auto& c : chroma) c.store (0.f);
        prepare (44100.0, 30.0);
    }

    void prepare (double sampleRate, double memorySeconds)
    {
        sr = sampleRate;
        setMemorySeconds (memorySeconds);

        // classe de hauteur de chaque bin FFT (-1 = ignoré)
        for (int k = 0; k < fftSize / 2; ++k)
        {
            const double f = k * sr / fftSize;
            if (f < 65.0 || f > 4200.0) { binPc[(size_t) k] = -1; continue; }
            const double midi = 69.0 + 12.0 * std::log2 (f / 440.0);
            const double nearest = std::round (midi);
            if (std::abs (midi - nearest) > 0.4) { binPc[(size_t) k] = -1; continue; }
            binPc[(size_t) k] = (((int) nearest % 12) + 12) % 12;
        }
        reset();
    }

    void setMemorySeconds (double s)
    {
        s = std::max (1.0, s);
        decay.store ((float) std::exp (-(double) hopSize / (sr * s)));
    }

    // Peut être appelé depuis n'importe quel thread.
    void requestReset() { resetFlag.store (true); }

    // Thread audio uniquement.
    void process (const float* mono, int numSamples)
    {
        if (resetFlag.exchange (false)) reset();

        for (int i = 0; i < numSamples; ++i)
        {
            frame[(size_t) writePos++] = mono[i];
            if (writePos == fftSize)
            {
                analyseFrame();
                std::copy (frame.begin() + hopSize, frame.end(), frame.begin());
                writePos = hopSize;
            }
        }
    }

    // Thread-safe (lecture depuis l'UI).
    std::array<float, 12> getChroma() const
    {
        std::array<float, 12> out {};
        for (size_t i = 0; i < 12; ++i) out[i] = chroma[i].load();
        return out;
    }

    static Result analyse (const std::array<float, 12>& c)
    {
        static const float major[12] = { 6.35f, 2.23f, 3.48f, 2.33f, 4.38f, 4.09f,
                                         2.52f, 5.19f, 2.39f, 3.66f, 2.29f, 2.88f };
        static const float minor[12] = { 6.33f, 2.68f, 3.52f, 5.38f, 2.60f, 3.53f,
                                         2.54f, 4.75f, 3.98f, 2.69f, 3.34f, 3.17f };
        Result r;
        float sum = 0.f;
        for (float v : c) sum += v;
        if (sum < 1e-4f) return r;

        const float meanC = sum / 12.f;
        float varC = 0.f;
        for (float v : c) varC += (v - meanC) * (v - meanC);
        if (varC < 1e-12f) return r;

        float best = -2.f, second = -2.f;
        int secondT = -1; bool secondMinor = false;
        for (int mode = 0; mode < 2; ++mode)
        {
            const float* p = mode ? minor : major;
            float meanP = 0.f;
            for (int i = 0; i < 12; ++i) meanP += p[i];
            meanP /= 12.f;
            float varP = 0.f;
            for (int i = 0; i < 12; ++i) varP += (p[i] - meanP) * (p[i] - meanP);

            for (int t = 0; t < 12; ++t)
            {
                float cov = 0.f;
                for (int i = 0; i < 12; ++i)
                    cov += (c[(size_t) ((t + i) % 12)] - meanC) * (p[i] - meanP);
                const float corr = cov / std::sqrt (varC * varP);

                r.scores[(size_t) (mode * 12 + t)] = corr;

                if (corr > best)
                {
                    second = best; secondT = r.tonic; secondMinor = r.minor;
                    best = corr; r.tonic = t; r.minor = (mode == 1);
                }
                else if (corr > second)
                {
                    second = corr; secondT = t; secondMinor = (mode == 1);
                }
            }
        }
        r.correlation = best;
        r.margin = best - second;
        r.altTonic = secondT;
        r.altMinor = secondMinor;
        r.altCorrelation = second;
        return r;
    }

    static const char* noteName (int pc)
    {
        static const char* n[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
        return n[((pc % 12) + 12) % 12];
    }

    static const char* noteNameFr (int pc)
    {
        static const char* n[12] = { "Do", "Do#", "Ré", "Ré#", "Mi", "Fa", "Fa#", "Sol", "Sol#", "La", "La#", "Si" };
        return n[((pc % 12) + 12) % 12];
    }

    // Notation Camelot (utilisée par Mixed In Key / DJ) : "8A", "11B"...
    static int camelotNumber (int tonic, bool minor)
    {
        static const int maj[12] = { 8, 3, 10, 5, 12, 7, 2, 9, 4, 11, 6, 1 };
        static const int min[12] = { 5, 12, 7, 2, 9, 4, 11, 6, 1, 8, 3, 10 };
        return minor ? min[tonic] : maj[tonic];
    }

private:
    void reset()
    {
        std::fill (frame.begin(), frame.end(), 0.f);
        writePos = 0;
        for (auto& c : acc) c = 0.f;
        for (auto& c : chroma) c.store (0.f);
    }

    void analyseFrame()
    {
        // niveau RMS : on ignore le silence
        double e = 0.0;
        for (float v : frame) e += (double) v * v;
        if (std::sqrt (e / fftSize) < 5e-4) { publish (true); return; }

        for (int i = 0; i < fftSize; ++i)
        {
            const int j = bitRev[(size_t) i];
            re[(size_t) j] = frame[(size_t) i] * window[(size_t) i];
            im[(size_t) j] = 0.f;
        }
        fft();

        std::array<float, 12> fc {};
        float total = 0.f;
        for (int k = 1; k < fftSize / 2; ++k)
        {
            const int pc = binPc[(size_t) k];
            if (pc < 0) continue;
            const float mag = std::sqrt (re[(size_t) k] * re[(size_t) k] + im[(size_t) k] * im[(size_t) k]);
            const float v = std::sqrt (mag);           // compression : limite le poids des basses
            fc[(size_t) pc] += v;
            total += v;
        }
        if (total <= 0.f) { publish (true); return; }

        const float d = decay.load();
        for (size_t i = 0; i < 12; ++i)
            acc[i] = acc[i] * d + fc[i] / total;       // trame normalisée (somme = 1)
        publish (false);
    }

    void publish (bool /*silent*/)
    {
        for (size_t i = 0; i < 12; ++i) chroma[i].store (acc[i]);
    }

    void fft()
    {
        for (int len = 2; len <= fftSize; len <<= 1)
        {
            const int half = len >> 1, step = fftSize / len;
            for (int i = 0; i < fftSize; i += len)
                for (int j = 0; j < half; ++j)
                {
                    const float wr = twCos[(size_t) (j * step)], wi = twSin[(size_t) (j * step)];
                    const size_t a = (size_t) (i + j), b = (size_t) (i + j + half);
                    const float tr = re[b] * wr - im[b] * wi;
                    const float ti = re[b] * wi + im[b] * wr;
                    re[b] = re[a] - tr; im[b] = im[a] - ti;
                    re[a] += tr;        im[a] += ti;
                }
        }
    }

    double sr = 44100.0;
    std::atomic<float> decay { 0.99f };
    std::atomic<bool>  resetFlag { false };

    std::vector<float> frame  = std::vector<float> (fftSize, 0.f);
    std::vector<float> window = std::vector<float> (fftSize, 0.f);
    std::vector<float> re = std::vector<float> (fftSize, 0.f), im = std::vector<float> (fftSize, 0.f);
    std::vector<float> twCos = std::vector<float> (fftSize / 2, 0.f), twSin = std::vector<float> (fftSize / 2, 0.f);
    std::vector<int>   bitRev = std::vector<int> (fftSize, 0);
    std::vector<int>   binPc  = std::vector<int> (fftSize / 2, -1);
    int writePos = 0;

    std::array<float, 12> acc {};
    std::array<std::atomic<float>, 12> chroma;
};
