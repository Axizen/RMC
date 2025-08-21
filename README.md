 # Cursed Angel

<div align="center">

![Cursed Angel Logo](https://via.placeholder.com/400x200/1a1a1a/ffffff?text=Cursed+Angel)

**3D Hack & Slash Platformer with Reactive Environment Combat**

[![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.5-0E1128?style=for-the-badge&logo=unrealengine)](https://www.unrealengine.com/)
[![C++](https://img.shields.io/badge/C++-17-00599C?style=for-the-badge&logo=cplusplus)](https://isocpp.org/)
[![License](https://img.shields.io/badge/License-MIT-green?style=for-the-badge)](LICENSE)
[![Status](https://img.shields.io/badge/Status-Prototype-orange?style=for-the-badge)]()

[🎮 **Play Demo**](#-quick-start) • [📖 **Documentation**](../../wiki) • [🐛 **Report Bug**](../../issues) • [💬 **Discussions**](../../discussions)

</div>

---

## 📖 Table of Contents

- [About](#-about)
- [Features](#-features)
- [Quick Start](#-quick-start)
- [Gameplay](#-gameplay)
- [Development](#-development)
- [Contributing](#-contributing)
- [License](#-license)

## 🎯 About

**Cursed Angel** is a 3D hack and slash platformer prototype that combines environmental interaction, companion AI, and transformation mechanics. Drawing inspiration from **Ratchet & Clank's** gadget-based exploration and **Devil May Cry's** stylish combat, the game features a unique "Source Code" narrative where players debug reality anomalies.

### 🎭 Story

Players control **Ripley**, a human with an angel's soul, accompanied by **Frank**, a fox with an angel's soul who is immune to curses. Both characters have been granted access to Yahweh's Source Code and tasked with debugging reality anomalies while maintaining secrecy from mortal authorities.

## ✨ Features

<table>
<tr>
<td width="50%">

### 🌍 **Reactive Environment System**
- Curse weapons interact with environmental elements
- Dynamic platform extension and barrier creation
- Hazard activation and environmental puzzles
- Real-time world state changes

</td>
<td width="50%">

### 🦊 **Intelligent Companion AI**
- Frank the fox provides utility support
- Telepathic communication system
- Witty personality with helpful abilities
- Autonomous utility station activation

</td>
</tr>
<tr>
<td width="50%">

### ⚡ **Cursed Angel Transformation**
- Devil Trigger-inspired ultimate ability
- Data point accumulation system
- Speed, damage, and fluidity bonuses
- Stunning visual effects and audio

</td>
<td width="50%">

### 🏟️ **Arena Combat System**
- Wave-based progression
- Environmental integration per wave
- Multiple enemy types with AI
- Progressive difficulty scaling

</td>
</tr>
</table>

## 🚀 Quick Start

### Prerequisites

```bash
# Required Software
- Unreal Engine 5.5+
- Visual Studio 2022 (C++ workload)
- Git with Git LFS
- Windows 10/11 (64-bit)
```

### Installation

1. **Clone the repository**
   ```bash
   git clone https://github.com/yourusername/cursed-angel.git
   cd cursed-angel
   ```

2. **Setup Git LFS**
   ```bash
   git lfs install
   git lfs pull
   ```

3. **Generate project files**
   ```bash
   # Right-click CursedAngel.uproject → "Generate Visual Studio project files"
   ```

4. **Open and compile**
   ```bash
   # Open CursedAngel.sln in Visual Studio
   # Build → Build Solution (Ctrl+Shift+B)
   # Open CursedAngel.uproject in Unreal Engine
   ```

### 🎮 First Play

1. Launch Unreal Engine and open the project
2. Press **Play** or open the `ArenaDemo` level
3. Use the controls below to start playing!

## 🎮 Gameplay

### 🎯 Controls

<table>
<tr>
<th>Input</th>
<th>Action</th>
<th>Input</th>
<th>Action</th>
</tr>
<tr>
<td><kbd>W</kbd><kbd>A</kbd><kbd>S</kbd><kbd>D</kbd></td>
<td>Movement</td>
<td><kbd>Mouse</kbd></td>
<td>Camera Look</td>
</tr>
<tr>
<td><kbd>Space</kbd></td>
<td>Jump</td>
<td><kbd>Left Click</kbd></td>
<td>Fire Curse Weapon</td>
</tr>
<tr>
<td><kbd>Right Click</kbd></td>
<td>Heavy Attack</td>
<td><kbd>1</kbd><kbd>2</kbd><kbd>3</kbd></td>
<td>Switch Curse Weapons</td>
</tr>
<tr>
<td><kbd>Q</kbd></td>
<td>Cursed Angel Transform</td>
<td><kbd>E</kbd></td>
<td>Request Frank Utility</td>
</tr>
</table>

### ⚔️ Curse Weapons

| Weapon | Key | Effect | Environmental Interaction |
|--------|-----|--------|---------------------------|
| **Corruption Rail** | <kbd>1</kbd> | High damage piercing shot | Extends platforms for traversal |
| **Fragment Needles** | <kbd>2</kbd> | Rapid-fire sticky projectiles | Activates hazard traps |
| **Void Mines** | <kbd>3</kbd> | Area denial explosives | Creates protective barriers |

### 🛠️ Frank's Utilities

| Utility | Effect | Duration | Cooldown |
|---------|--------|----------|----------|
| **Curse Amplifier** | 2x weapon damage | 10s | 15s |
| **Element Highlighter** | Shows interactive elements | 8s | 12s |
| **Emergency Shield** | 50% damage reduction | 8s | 20s |

## 🏗️ Development

### 📁 Project Structure

```
CursedAngel/
├── 📁 Content/
│   ├── 📁 Characters/           # Player and companion assets
│   │   ├── 📁 Ripley/          # Player character
│   │   └── 📁 Frank/           # Companion fox
│   ├── 📁 CurseWeapons/        # Weapon blueprints and effects
│   ├── 📁 Environment/         # Arena and reactive elements
│   ├── 📁 UI/                  # HUD and interface
│   └── 📁 Audio/               # Sound effects and music
├── 📁 Source/CursedAngel/      # C++ source code
│   ├── 📄 CurseWeaponComponent.*
│   ├── 📄 CursedAngelComponent.*
│   ├── 📄 FrankAI.*
│   ├── 📄 FrankAIController.*
│   └── 📄 ReactiveArenaElement.*
├── 📁 Config/                  # Engine configuration
└── 📁 Plugins/                 # Third-party plugins
```

### 🔧 Key Systems

<details>
<summary><strong>🌍 Reactive Environment System</strong></summary>

```cpp
UCLASS(BlueprintType, Blueprintable)
class CURSEDANGEL_API AReactiveArenaElement : public AActor
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reactive")
    ECurseWeaponType ReactsToWeapon;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reactive")
    EReactionType ReactionBehavior;
    
    UFUNCTION(BlueprintCallable, Category = "Reactive")
    void TriggerReaction(ECurseWeaponType CurseType, FVector ImpactLocation);
};
```
</details>

<details>
<summary><strong>🦊 Frank AI Controller</strong></summary>

```cpp
UCLASS(BlueprintType, Blueprintable)
class CURSEDANGEL_API AFrankAIController : public AAIController
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Frank AI")
    void SetAIState(EFrankAIState NewState);
    
    UFUNCTION(BlueprintCallable, Category = "Frank AI")
    void ActivateNearestUtility();

private:
    void UpdateFollowBehavior(float DeltaTime);
    void UpdateUtilityDetection();
};
```
</details>

<details>
<summary><strong>⚡ Cursed Angel Transformation</strong></summary>

```cpp
UCLASS(BlueprintType, Blueprintable)
class CURSEDANGEL_API UCursedAngelComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Angel")
    float DataPoints = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Angel")
    float SpeedMultiplier = 1.5f;
    
    UFUNCTION(BlueprintCallable, Category = "Cursed Angel")
    void ActivateTransformation();
};
```
</details>

### 🔨 Building from Source

<details>
<summary><strong>Development Build</strong></summary>

1. **Prerequisites**
   ```bash
   # Install Visual Studio 2022 with C++ workload
   # Install Unreal Engine 5.5
   # Ensure Git LFS is configured
   ```

2. **Compile C++ Classes**
   ```bash
   # Method 1: Visual Studio
   Open CursedAngel.sln → Build → Build Solution

   # Method 2: Unreal Engine
   Open project → Compile button in toolbar
   ```

3. **Package for Distribution**
   ```bash
   # In Unreal Editor
   File → Package Project → Windows (64-bit)
   # Select output directory
   # Wait for packaging to complete
   ```
</details>

### 📊 Performance Targets

| Metric | Target | Current |
|--------|--------|---------|
| **Frame Rate** | 60 FPS | ✅ 60+ FPS |
| **Memory Usage** | < 4GB RAM | ✅ ~3.2GB |
| **Loading Time** | < 10s | ✅ ~7s |
| **Package Size** | < 2GB | ✅ ~1.8GB |

## 🗺️ Roadmap

### ✅ **Completed (v0.1.0 - Prototype)**
- [x] Core curse weapon system with environmental interactions
- [x] Frank companion AI with utility support
- [x] Cursed Angel transformation with visual effects
- [x] Arena combat with wave-based progression
- [x] Complete UI system with HUD and feedback

### 🚧 **In Progress (v0.2.0)**
- [ ] Enhanced Frank personality system with more dialogue
- [ ] Additional curse weapon types and combinations
- [ ] Advanced enemy AI with varied behaviors
- [ ] Performance optimizations and bug fixes

### 📅 **Planned (v0.3.0+)**
- [ ] Story mode with narrative progression
- [ ] Multiple arena environments with unique mechanics
- [ ] Save system and progression tracking
- [ ] Steam integration and achievements

## 🤝 Contributing

We welcome contributions from the community! Please read our [Contributing Guidelines](CONTRIBUTING.md) before getting started.

### 🚀 How to Contribute

1. **🍴 Fork the repository**
2. **🌿 Create a feature branch**
   ```bash
   git checkout -b feature/amazing-feature
   ```
3. **💻 Make your changes**
4. **🧪 Test thoroughly**
5. **📝 Commit with clear messages**
   ```bash
   git commit -m 'feat: add amazing feature'
   ```
6. **📤 Push to your branch**
   ```bash
   git push origin feature/amazing-feature
   ```
7. **🔄 Open a Pull Request**

### 📋 Development Guidelines

- Follow [Unreal Engine C++ Coding Standards](https://docs.unrealengine.com/5.3/en-US/epic-cplusplus-coding-standards-for-unreal-engine/)
- Use meaningful commit messages ([Conventional Commits](https://www.conventionalcommits.org/))
- Test all changes in both Editor and packaged builds
- Update documentation for new features

## 🐛 Known Issues

<details>
<summary><strong>Current Known Issues</strong></summary>

| Issue | Severity | Status |
|-------|----------|--------|
| Frank occasionally gets stuck on complex geometry | Low | 🔍 Investigating |
| Transformation effects may persist after death | Medium | 🔧 In Progress |
| Audio cues can overlap during rapid weapon switching | Low | 📋 Planned |
| UI scaling issues on ultrawide monitors | Low | 📋 Planned |

See our [Issue Tracker](../../issues) for the complete list and to report new bugs.
</details>

## 📄 License

This project is licensed under the **MIT License** - see the [LICENSE](LICENSE) file for details.

### 🎨 Third-Party Assets

<details>
<summary><strong>Asset Acknowledgments</strong></summary>

| Asset Pack | Usage | License |
|------------|-------|---------|
| **StylishCombatStarterKit** | Melee combat foundation | Asset Store License |
| **Cat Girl Character Pack** | Player character mesh | Asset Store License |
| **Weapon Asset Packs** | Railgun, Needler, ProximityMineLauncher | Asset Store License |
| **Unreal Engine Starter Content** | Environmental assets | Epic Games License |

</details>

## 🙏 Acknowledgments

### 🎮 **Inspiration**
- **Ratchet & Clank** - Gadget-based environmental interaction
- **Devil May Cry** - Stylish combat and transformation mechanics
- **.hack franchise** - Digital reality narrative themes
- **MASH (1970)** - Character personality and dialogue tone

### 👥 **Special Thanks**
- Epic Games for Unreal Engine 5
- The Unreal Engine community for tutorials and support
- Asset creators for high-quality content packs

---

<div align="center">

### 📞 **Support & Community**

[![Discord](https://img.shields.io/badge/Discord-Community-7289da?style=for-the-badge&logo=discord)](https://discord.gg/cursedangel)
[![Documentation](https://img.shields.io/badge/Docs-Wiki-blue?style=for-the-badge&logo=gitbook)](../../wiki)
[![Issues](https://img.shields.io/badge/Issues-Bug%20Reports-red?style=for-the-badge&logo=github)](../../issues)
[![Discussions](https://img.shields.io/badge/Discussions-Q%26A-green?style=for-the-badge&logo=github)](../../discussions)

### 📈 **Project Stats**

![GitHub repo size](https://img.shields.io/github/repo-size/Axizen/cursed-angel?style=flat-square)
![GitHub contributors](https://img.shields.io/github/contributors/Axizen/cursed-angel?style=flat-square)
![GitHub last commit](https://img.shields.io/github/last-commit/Axizen/cursed-angel?style=flat-square)
![GitHub issues](https://img.shields.io/github/issues/Axizen/cursed-angel?style=flat-square)
![GitHub pull requests](https://img.shields.io/github/issues-pr/Axizen/cursed-angel?style=flat-square)
![GitHub stars](https://img.shields.io/github/stars/Axizen/cursed-angel?style=flat-square)

---

**⭐ Star this repository if you found it helpful!**

*"Flippin' amazing."*

</div>
