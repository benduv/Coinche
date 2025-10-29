$imagemagick = "C:\Program Files\ImageMagick-7.1.2-Q16-HDRI\magick.exe"
$sourcePath = "C:\Users\33672\Downloads\cartes-pdf"
$destPath = "C:\Users\33672\projects\Coinche\resources\cards"

# Table de correspondance pour les couleurs
$suitMap = @{
    "coeur" = "coeur"
    "carreau" = "carreau"
    "pique" = "pique"
    "trefle" = "trefle"
}

# Table de correspondance pour les valeurs
$valueMap = @{
    "01" = "14" # As
    "02" = "2"
    "03" = "3"
    "04" = "4"
    "05" = "5"
    "06" = "6"
    "07" = "7"
    "08" = "8"
    "09" = "9"
    "10" = "10"
    "V" = "11"  # Valet
    "D" = "12"  # Dame
    "R" = "13"  # Roi
}

# Crée le dossier de destination s'il n'existe pas
if (-not (Test-Path $destPath)) {
    New-Item -ItemType Directory -Path $destPath
}

# Convertit chaque carte
Get-ChildItem $sourcePath -Filter "*.pdf" | ForEach-Object {
    $filename = $_.BaseName
    
    # Ignore les fichiers spéciaux comme dos-bleu.pdf et joker-*.pdf
    if ($filename -match "^(dos-|joker-)") {
        return
    }
    
    # Extrait la valeur et la couleur du nom de fichier
    if ($filename -match "^([^-]+)-(.+)$") {
        $value = $matches[1]
        $suit = $matches[2]
        
        if ($valueMap.ContainsKey($value) -and $suitMap.ContainsKey($suit)) {
            $newValue = $valueMap[$value]
            $newSuit = $suitMap[$suit]
            $newName = "${newSuit}_${newValue}.png"
            $outputPath = Join-Path $destPath $newName
            
            Write-Host "Converting $($_.Name) to $newName..."
            & $imagemagick $_.FullName -density 300 -resize 200x300 $outputPath
        }
    }
}

# Convertir le dos de carte
$dosSource = Join-Path $sourcePath "dos-bleu.pdf"
if (Test-Path $dosSource) {
    $dosOutput = Join-Path $destPath "back.png"
    Write-Host "Converting dos-bleu.pdf to back.png..."
    & $imagemagick $dosSource -density 300 -resize 200x300 $dosOutput
}