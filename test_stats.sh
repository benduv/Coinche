#!/bin/bash

# Script de test pour le système de statistiques quotidiennes
# Usage: ./test_stats.sh [mot_de_passe_smtp]

echo "======================================"
echo "Test du système de statistiques"
echo "======================================"
echo ""

# Vérifier que la base de données existe
if [ ! -f "coinche.db" ]; then
    echo "❌ Erreur: coinche.db n'existe pas"
    echo "   Veuillez démarrer le serveur une première fois pour créer la base"
    exit 1
fi

echo "✅ Base de données trouvée"
echo ""

# Vérifier que la table daily_stats existe
TABLE_EXISTS=$(sqlite3 coinche.db "SELECT name FROM sqlite_master WHERE type='table' AND name='daily_stats';" 2>/dev/null)

if [ -z "$TABLE_EXISTS" ]; then
    echo "❌ Erreur: La table daily_stats n'existe pas"
    echo "   Veuillez recompiler et redémarrer le serveur"
    exit 1
fi

echo "✅ Table daily_stats trouvée"
echo ""

# Afficher la structure de la table
echo "📋 Structure de la table daily_stats:"
sqlite3 coinche.db ".schema daily_stats"
echo ""

# Afficher les données actuelles
echo "📊 Données actuelles:"
sqlite3 coinche.db "SELECT date, logins, game_rooms_created, new_accounts, player_quits FROM daily_stats ORDER BY date DESC LIMIT 5;" -header -column
echo ""

# Insérer des données de test pour aujourd'hui
TODAY=$(date +%Y-%m-%d)
echo "🧪 Insertion de données de test pour $TODAY..."

sqlite3 coinche.db <<EOF
INSERT OR REPLACE INTO daily_stats (date, logins, game_rooms_created, new_accounts, player_quits)
VALUES ('$TODAY', 45, 12, 3, 5);
EOF

echo "✅ Données de test insérées"
echo ""

# Insérer des données pour hier (pour comparaison)
YESTERDAY=$(date -d "yesterday" +%Y-%m-%d 2>/dev/null || date -v-1d +%Y-%m-%d 2>/dev/null)
echo "🧪 Insertion de données de test pour hier ($YESTERDAY)..."

sqlite3 coinche.db <<EOF
INSERT OR REPLACE INTO daily_stats (date, logins, game_rooms_created, new_accounts, player_quits)
VALUES ('$YESTERDAY', 39, 10, 0, 6);
EOF

echo "✅ Données de comparaison insérées"
echo ""

# Afficher les données mises à jour
echo "📊 Données après insertion:"
sqlite3 coinche.db "SELECT date, logins, game_rooms_created, new_accounts, player_quits FROM daily_stats ORDER BY date DESC LIMIT 5;" -header -column
echo ""

# Test d'envoi d'email (nécessite le mot de passe SMTP)
SMTP_PASSWORD="$1"
if [ -z "$SMTP_PASSWORD" ]; then
    SMTP_PASSWORD="$COINCHE_SMTP_PASSWORD"
fi

if [ -z "$SMTP_PASSWORD" ]; then
    echo "⚠️  Mot de passe SMTP non fourni"
    echo "   Pour tester l'envoi d'email, relancez avec:"
    echo "   ./test_stats.sh <mot_de_passe_smtp>"
    echo "   ou définissez: export COINCHE_SMTP_PASSWORD=<mot_de_passe>"
else
    echo "📧 Test d'envoi d'email..."
    echo "   (L'email sera envoyé à contact@nebuludik.fr)"
    echo ""
    echo "   Pour tester l'envoi, démarrez le serveur avec:"
    echo "   ./server --smtp-password \"$SMTP_PASSWORD\""
    echo ""
    echo "   Puis dans le code (server_main.cpp), ajoutez après la ligne 'GameServer server(...):':"
    echo "   QTimer::singleShot(5000, [&server]() { server.getStatsReporter()->sendDailyReport(); });"
fi

echo ""
echo "======================================"
echo "✅ Test terminé avec succès!"
echo "======================================"
echo ""
echo "📝 Prochaines étapes:"
echo "1. Recompiler le serveur: cd build && cmake .. && make"
echo "2. Démarrer le serveur avec le mot de passe SMTP:"
echo "   ./server --smtp-password \"<votre_mot_de_passe>\""
echo "3. Le rapport sera envoyé automatiquement chaque jour à minuit"
echo "4. Pour tester immédiatement, voir STATS_README.md"
echo ""
