# EXSA — Discovery 2026  
Firmware du module EXSA (Extension Satellite Autonome)  
Architecture modulaire, non bloquante, orientée protocole SA ↔ EXSA.

---

## 📌 Présentation générale

Le module **EXSA** est un satellite autonome chargé de gérer :

- la signalisation SNCF (aspects, transitions, clignotements)
- les aiguilles (servos + micro-switchs)
- les LED directionnelles
- le comptage essieux (quadrature)
- le canton (LEDs occupation + mouvement)
- la communication RS485 avec le SA
- le **Booster DCC** (si présent) : courant, tension, cutout RailCom, occupation

L’architecture Discovery 2026 sépare clairement :

| Domaine | Module |
|--------|--------|
| Logique locale | `EXSA_Runtime` |
| Initialisation | `EXSA_System` |
| Protocole RS485 | `EXSA_UartRx` / `EXSA_UartTx` |
| Booster | `EXSA_BoosterCore` |
| Calibration | `EXSA_Calibration` |
| Signaux | `EXSA_Signaux` |
| Servos | `EXSA_Servo` |
| Canton | `EXSA_Canton` |

---

## 🧩 Architecture générale

```text
EXSA_Main
├── EXSA_System      → Initialisation complète
├── EXSA_Runtime     → Boucle temps réel (hors booster)
├── EXSA_UartRx      → Parsing trames SA → EXSA
├── EXSA_UartTx      → Envoi trames EXSA → SA
├── EXSA_BoosterCore → Tâche FreeRTOS (DCC, courant, RailCom)
├── EXSA_Calibration → Machine à états non bloquante
├── EXSA_Signaux     → Gestion aspects SNCF
├── EXSA_Servo       → Servos + anti-blocage
├── EXSA_Canton      → LEDs canton + mouvement
└── EXSA_Quadrature  → Comptage essieux
```
---

## 🔌 Protocole SA ↔ EXSA (RS485)

Toutes les trames suivent le format :

[0xAA][OPCODE][DATA...]

### 🟦 SA → EXSA

| Opcode | Fonction |
|--------|----------|
| `32` | PING |
| `E4` | Topologie CAN |
| `E5` | Config signaux |
| `E6` | Aspect horaire |
| `E7` | Aspect antihoraire |
| `E8` | Direction horaire |
| `E9` | Direction antihoraire |
| `EA` | **Occupation voisins** |
| `F0` | **Servo move** |
| `F1` | **Servo config** |
| `F2` | **Servo test** |
| `F3` | **Recalibration Booster** |
| `F4` | **Application seuils calibrés** |
| `F5` | **Booster ON/OFF** |

### 🟩 EXSA → SA

| Opcode | Fonction |
|--------|----------|
| `03` | Ponctuel |
| `04` | Occupation |
| `05` | Delta axe |
| `06` | Position aiguille |
| `07` | Booster (courant, tension, état) |
| `08` | **RailCom adresse** |
| `09` | **Seuils calibrés (libre/occupé)** |

---

## ⚡ Calibration Booster (Discovery 2026)

La calibration du courant est désormais :

- **non bloquante**
- **gérée par EXSA_BoosterCore**
- **pilotée par le SA**
- **persistante via JSON côté SA**

### 🔁 Cycle complet

1. **SA → EXSA : F3**  
   Demande de recalibration automatique.

2. **EXSA → SA : 09**  
   Envoi des seuils calibrés :  
   - seuilLibre  
   - seuilOccupe  

3. **SA stocke dans settings.json**

4. **SA → EXSA : F4**  
   Restaure les seuils au démarrage.

5. **EXSA_Calibration::setSeuils()**  
   Application immédiate.

---

## 🏗️ EXSA_System (initialisation)

- Lecture DIP (horaire / booster)
- Initialisation :
  - PCA9685
  - MCP23017
  - Quadrature (ISR + queue)
  - Signaux
  - Canton
  - Servos
- Démarrage tâche BoosterCore (si présent)
- Initialisation EXSA_Calibration

---

## 🔄 EXSA_Runtime (boucle temps réel)

- Lecture quadrature (queue FreeRTOS)
- Gestion essieux (delta + ponctuel)
- Mise à jour signaux SNCF
- Mise à jour canton (LEDs + mouvement)
- Anti-blocage servos
- **Aucune gestion booster ici**

---

## 🧪 EXSA_Calibration

Machine à états non bloquante :

- IDLE  
- SAMPLE_LIBRE  
- SAMPLE_OCCUPE  
- COMPUTE  
- SEND_RESULT (trame 09)

---

## 📁 Organisation du code

```text
src/
├── EXSA_Main.*
├── EXSA_System.*
├── EXSA_Runtime.*
├── EXSA_UartRx.*
├── EXSA_UartTx.*
├── EXSA_BoosterCore.*
├── EXSA_Calibration.*
├── EXSA_Signaux.*
├── EXSA_Servo.*
├── EXSA_Canton.*
├── EXSA_Quadrature.*
├── EXSA_Switches.*
└── Discovery_Protocol.h
```
---

## 📝 Licence

Projet personnel — usage libre pour développement, test et documentation.

---

## 👤 Auteur

Développé par **Bruno**  
Architecture Discovery 2026 — modules EXSA / SA / Booster# Carte_AH_Canton
