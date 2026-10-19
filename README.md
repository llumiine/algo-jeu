# algo-jeu


### Installer Git

```powershell
winget install --id Git.Git -e --source winget
```

Fermez complètement VS Code, rouvrez-le, puis vérifiez :

```powershell
git --version
```

Configurez votre identité (avec l'email de votre compte GitHub) :

```powershell
git config --global user.name "Votre Prénom"
git config --global user.email "votre-email-github@exemple.com"
```

### Installer le compilateur C (gcc)

```powershell
winget install BrechtSanders.WinLibs.POSIX.UCRT
```

Fermez complètement VS Code, rouvrez-le, puis vérifiez :

```powershell
gcc --version
```

**Si `gcc` n'est pas reconnu**, copiez ces lignes ensemble dans PowerShell.
Elles retrouvent gcc et l'ajoutent au PATH de Windows de façon permanente :

```powershell
$gcc = Get-ChildItem "$env:LOCALAPPDATA\Microsoft\WinGet\Packages" -Recurse -Filter gcc.exe -ErrorAction SilentlyContinue | Select-Object -First 1
$bin = $gcc.DirectoryName
$cheminUtilisateur = [Environment]::GetEnvironmentVariable("Path", "User")
[Environment]::SetEnvironmentVariable("Path", "$cheminUtilisateur;$bin", "User")
$env:Path += ";$bin"
gcc --version
```

---