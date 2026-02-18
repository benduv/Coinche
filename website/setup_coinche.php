<?php
/**
 * ============================================================
 *  COINCHE DE L'ESPACE — Déploiement du site WordPress
 *  Version navigateur — idempotent (crée ou met à jour)
 * ============================================================
 * 1. Uploadez ce fichier dans www/ sur l'hébergement OVH
 * 2. Ouvrez dans le navigateur : http://nebuludik.fr/setup_coinche.php
 * 3. Cliquez "Déployer"
 * 4. Relancez à chaque fois que vous modifiez le texte
 * ============================================================
 */

// ── Si pas de POST → afficher le bouton de confirmation ──
if ( $_SERVER['REQUEST_METHOD'] !== 'POST' || ! isset( $_POST['coinche_run'] ) ) {
?>
<!DOCTYPE html>
<html lang="fr">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Setup — Coinche de l'Espace</title>
<style>
    body        { background:#0a0a1a; color:#ccc; font-family:'Segoe UI',sans-serif; padding:60px 24px; margin:0; }
    .box        { max-width:580px; margin:0 auto; background:#111133; border:1px solid #222244; border-radius:10px; padding:40px; }
    h1          { color:#FFD700; margin-top:0; }
    ul          { line-height:2; }
    .btn        { background:#FFD700; color:#0a0a1a; border:none; padding:14px 36px; font-size:1.1em; font-weight:bold; border-radius:6px; cursor:pointer; }
    .btn:hover  { background:#ffe54d; }
    .warn       { color:#f88; margin-top:20px; }
</style>
</head>
<body>
<div class="box">
    <h1>Coinche de l'Espace — Déploiement</h1>
    <p>Ce script va créer ou mettre à jour :</p>
    <ul>
        <li>Page <strong>Présentation</strong> (page d'accueil)</li>
        <li>Page <strong>Règles du Jeu</strong></li>
        <li>Page <strong>Politique de Confidentialité</strong></li>
        <li>Page <strong>Contact</strong></li>
        <li>Menu de navigation</li>
    </ul>
    <p class="warn">Pages existantes → <strong>mises à jour</strong>. Pages absentes → <strong>créées</strong>.</p>
    <form method="POST">
        <input type="hidden" name="coinche_run" value="1">
        <button type="submit" class="btn">Déployer</button>
    </form>
</div>
</body>
</html>
<?php
    exit;
}

// ============================================================
// ── Charger WordPress ──
// ============================================================
require_once( dirname( __FILE__ ) . '/wp-load.php' );

// ── Fonctions de sortie HTML ──
function coinche_ok( $msg )  { echo "<p style='color:#4caf50;margin:6px 0;'><strong>✓</strong> $msg</p>"; }
function coinche_err( $msg ) { echo "<p style='color:#f44336;margin:6px 0;'><strong>✗</strong> $msg</p>"; }
function coinche_log( $msg ) { echo "<p style='color:#aaa;margin:6px 0;'>→ $msg</p>"; }

// ── Crée ou met à jour une page selon son slug ──
function coinche_upsert_page( $title, $slug, $content ) {
    $existing = get_page_by_path( $slug, OBJECT, 'page' );
    if ( $existing ) {
        $result = wp_update_post( [
            'ID'           => $existing->ID,
            'post_title'   => $title,
            'post_content' => $content,
            'post_status'  => 'publish',
        ], true );
        if ( is_wp_error( $result ) ) return $result;
        coinche_ok( "$title mise à jour (ID: $result)" );
        return $result;
    }
    $result = wp_insert_post( [
        'post_title'   => $title,
        'post_name'    => $slug,
        'post_status'  => 'publish',
        'post_type'    => 'page',
        'post_content' => $content,
    ], true );
    if ( is_wp_error( $result ) ) return $result;
    coinche_ok( "$title créée (ID: $result)" );
    return $result;
}

// ── En-tête HTML résultat ──
echo '<!DOCTYPE html><html lang="fr"><head><meta charset="utf-8">';
echo '<title>Setup — Coinche de l\'Espace</title>';
echo '<style>body{background:#0a0a1a;color:#ccc;font-family:"Segoe UI",sans-serif;padding:60px 24px;margin:0;}';
echo '.box{max-width:680px;margin:0 auto;background:#111133;border:1px solid #222244;border-radius:10px;padding:40px;}';
echo 'h1,h2{color:#FFD700;} a{color:#FFD700;}</style></head>';
echo '<body><div class="box"><h1>Coinche de l\'Espace — Résultat</h1>';


// ============================================================
// A — PAGE PRÉSENTATION (sera la page d'accueil)
// ============================================================
$content_a = <<<'__END_A__'
<!-- wp:html -->
<div style="max-width:860px;margin:0 auto;padding:50px 24px;font-family:'Segoe UI',Tahoma,sans-serif;color:#cccccc;line-height:1.7;">

  <h1 style="color:#FFD700;text-align:center;font-size:2.8em;margin:0 0 8px 0;letter-spacing:1px;">Coinche de l'Espace</h1>
  <p style="color:#aaa;text-align:center;font-style:italic;font-size:1.2em;margin:0 0 50px 0;">Un jeu de coinche, enfin fait comme il faut.</p>

  <h3 style="color:#FFD700;font-size:1.5em;border-bottom:1px solid #333;padding-bottom:8px;">Notre histoire</h3>
  <p>Vous avez déjà essayé de jouer à la coinche sur mobile ? Vous connaissez alors très bien la frustration qui accompagne la grande majorité des applications existantes. Entre des systèmes artificiels, des interfaces épuisantes à naviguer et des publicités omniprésentes, l'expérience de jeu est loin d'être satisfaisante.</p>
  <p>C'est cette frustration qui a poussé un passionné de coinche à développer <strong style="color:#FFD700;">Coinche de l'Espace</strong> — une application qui remet enfin le plaisir de jouer au centre de l'expérience.</p>

  <h3 style="color:#FFD700;font-size:1.5em;border-bottom:1px solid #333;padding-bottom:8px;margin-top:40px;">Ce qui nous a frustré dans les autres applications</h3>
  <ul style="list-style:none;padding:0;margin:0;">
    <li style="margin-bottom:18px;padding-left:20px;border-left:3px solid #FFD700;">
      <strong style="color:#FFD700;">1. Le système de jetons</strong><br>
      Des jetons virtuels forcés, des achats intégrés pour pouvoir jouer — le jeu devient une boutique avant d'être un jeu.
    </li>
    <li style="margin-bottom:18px;padding-left:20px;border-left:3px solid #FFD700;">
      <strong style="color:#FFD700;">2. L'impossibilité de coincher à la volée</strong><br>
      Dans une vraie partie de coinche, la coinche peut être annoncée immédiatement après une enchère. Les applications existantes vous forcent à attendre votre tour, ce qui tue le dynamisme du jeu.
    </li>
    <li style="margin-bottom:18px;padding-left:20px;border-left:3px solid #FFD700;">
      <strong style="color:#FFD700;">3. Les publicités envahissantes</strong><br>
      Une pub avant chaque manche, après chaque manche, parfois même pendant la partie. Le plaisir de jouer est constamment interrompu.
    </li>
    <li style="margin-bottom:18px;padding-left:20px;border-left:3px solid #FFD700;">
      <strong style="color:#FFD700;">4. Des interfaces parfois vieillottes</strong><br>
      Des designs qui semblent sortis des années 2010, des boutons flous, des animations inexistantes — on peut mieux faire.
    </li>
  </ul>

  <h3 style="color:#FFD700;font-size:1.5em;border-bottom:1px solid #333;padding-bottom:8px;margin-top:40px;">Notre vision</h3>
  <p>Coinche de l'Espace est un jeu de coinche multijoueur en ligne, développé avec passion et sans compromis sur l'expérience de jeu. Pas de jetons, pas de publicités intrusives, une coinche à la volée comme dans la vie, et une interface moderne inspirée par l'immensité de l'espace.</p>

  <h3 style="color:#FFD700;font-size:1.5em;border-bottom:1px solid #333;padding-bottom:8px;margin-top:40px;">Aperçu du jeu</h3>
  <p style="color:#666;font-style:italic;">[Screenshots à ajouter — éditez cette page dans l'admin WordPress pour insérer vos images]</p>

  <h3 style="color:#FFD700;font-size:1.5em;border-bottom:1px solid #333;padding-bottom:8px;margin-top:40px;">Gameplay</h3>
  <p style="color:#666;font-style:italic;">[Vidéos à ajouter — éditez cette page dans l'admin WordPress pour insérer vos enregistrements]</p>

  <h3 style="color:#FFD700;font-size:1.5em;border-bottom:1px solid #333;padding-bottom:8px;margin-top:40px;">Télécharger le jeu</h3>
  <p>Coinche de l'Espace est disponible sur <strong>Windows</strong> et <strong>Android</strong>.</p>
  <div style="display:flex;gap:16px;flex-wrap:wrap;margin:24px 0;">
    <a href="#" style="display:inline-block;background:#FFD700;color:#0a0a1a;padding:14px 28px;border-radius:8px;font-weight:bold;text-decoration:none;font-size:1.05em;">⬇ Windows (.exe)</a>
    <a href="#" style="display:inline-block;background:#FFD700;color:#0a0a1a;padding:14px 28px;border-radius:8px;font-weight:bold;text-decoration:none;font-size:1.05em;">⬇ Android (.apk)</a>
  </div>
  <p style="color:#666;font-size:0.85em;font-style:italic;">Les liens de téléchargement seront mis à jour très prochainement.</p>
</div>
<!-- /wp:html -->
__END_A__;

$id_a = coinche_upsert_page( 'Présentation', 'presentation', $content_a );
if ( is_wp_error( $id_a ) ) coinche_err( 'Présentation : ' . $id_a->get_error_message() );


// ============================================================
// B — PAGE RÈGLES DU JEU
// ============================================================
$content_b = <<<'__END_B__'
<!-- wp:html -->
<div style="max-width:860px;margin:0 auto;padding:50px 24px;font-family:'Segoe UI',Tahoma,sans-serif;color:#cccccc;line-height:1.7;">

  <h1 style="color:#FFD700;text-align:center;font-size:2.5em;margin:0 0 40px 0;">Règles de la Coinche</h1>

  <!-- ─── BUT DU JEU ─── -->
  <h2 style="color:#FFD700;font-size:1.8em;border-bottom:2px solid #FFD700;padding-bottom:8px;margin-top:40px;">But du jeu</h2>
  <p>La Coinche est un jeu de cartes qui se joue à 4 joueurs répartis en 2 équipes de 2. L'objectif est de réaliser le contrat annoncé lors des enchères en marquant un maximum de points.</p>

  <!-- ─── LES CARTES ─── -->
  <h2 style="color:#FFD700;font-size:1.8em;border-bottom:2px solid #FFD700;padding-bottom:8px;margin-top:40px;">Les cartes et leur valeur</h2>
  <p>Le jeu comprend 32 cartes (7, 8, 9, 10, Valet, Dame, Roi, As) dans 4 couleurs (Pique, Cœur, Carreau, Trèfle).</p>

  <h3 style="color:#FFD700;margin-top:28px;">Valeur des cartes à l'atout</h3>
  <table style="width:100%;border-collapse:collapse;margin:10px 0 24px 0;background:#111133;">
    <tr style="background:#1a1a3a;"><th style="padding:10px 14px;border:1px solid #333366;color:#FFD700;text-align:left;">Carte</th><th style="padding:10px 14px;border:1px solid #333366;color:#FFD700;text-align:left;">Points</th></tr>
    <tr><td style="padding:8px 14px;border:1px solid #333366;">Valet</td><td style="padding:8px 14px;border:1px solid #333366;"><strong style="color:#FFD700;">20</strong></td></tr>
    <tr><td style="padding:8px 14px;border:1px solid #333366;">9</td><td style="padding:8px 14px;border:1px solid #333366;"><strong style="color:#FFD700;">14</strong></td></tr>
    <tr><td style="padding:8px 14px;border:1px solid #333366;">As</td><td style="padding:8px 14px;border:1px solid #333366;">11</td></tr>
    <tr><td style="padding:8px 14px;border:1px solid #333366;">10</td><td style="padding:8px 14px;border:1px solid #333366;">10</td></tr>
    <tr><td style="padding:8px 14px;border:1px solid #333366;">Roi</td><td style="padding:8px 14px;border:1px solid #333366;">4</td></tr>
    <tr><td style="padding:8px 14px;border:1px solid #333366;">Dame</td><td style="padding:8px 14px;border:1px solid #333366;">3</td></tr>
    <tr><td style="padding:8px 14px;border:1px solid #333366;">8</td><td style="padding:8px 14px;border:1px solid #333366;">0</td></tr>
    <tr><td style="padding:8px 14px;border:1px solid #333366;">7</td><td style="padding:8px 14px;border:1px solid #333366;">0</td></tr>
  </table>

  <h3 style="color:#FFD700;margin-top:28px;">Valeur des cartes hors atout</h3>
  <table style="width:100%;border-collapse:collapse;margin:10px 0 24px 0;background:#111133;">
    <tr style="background:#1a1a3a;"><th style="padding:10px 14px;border:1px solid #333366;color:#FFD700;text-align:left;">Carte</th><th style="padding:10px 14px;border:1px solid #333366;color:#FFD700;text-align:left;">Points</th></tr>
    <tr><td style="padding:8px 14px;border:1px solid #333366;">As</td><td style="padding:8px 14px;border:1px solid #333366;">11</td></tr>
    <tr><td style="padding:8px 14px;border:1px solid #333366;">10</td><td style="padding:8px 14px;border:1px solid #333366;">10</td></tr>
    <tr><td style="padding:8px 14px;border:1px solid #333366;">Roi</td><td style="padding:8px 14px;border:1px solid #333366;">4</td></tr>
    <tr><td style="padding:8px 14px;border:1px solid #333366;">Dame</td><td style="padding:8px 14px;border:1px solid #333366;">3</td></tr>
    <tr><td style="padding:8px 14px;border:1px solid #333366;">Valet</td><td style="padding:8px 14px;border:1px solid #333366;">2</td></tr>
    <tr><td style="padding:8px 14px;border:1px solid #333366;">9, 8, 7</td><td style="padding:8px 14px;border:1px solid #333366;">0</td></tr>
  </table>

  <!-- ─── LES ENCHÈRES ─── -->
  <h2 style="color:#FFD700;font-size:1.8em;border-bottom:2px solid #FFD700;padding-bottom:8px;margin-top:40px;">Les enchères</h2>
  <p>Chaque joueur, à tour de rôle, peut :</p>
  <ul>
    <li>Annoncer un contrat (nombre de points qu'il pense réaliser) et choisir l'atout</li>
    <li>Passer</li>
  </ul>
  <p>Les contrats vont de <strong>80 à 160 points</strong> par paliers de 10.</p>

  <h3 style="color:#FFD700;margin-top:28px;">Annonces spéciales</h3>
  <ul style="list-style:none;padding:0;">
    <li style="margin-bottom:14px;padding-left:16px;border-left:3px solid #FFD700;"><strong style="color:#FFD700;">Coinche</strong> — Double les points du contrat adverse si vous pensez qu'ils ne réussiront pas.</li>
    <li style="margin-bottom:14px;padding-left:16px;border-left:3px solid #FFD700;"><strong style="color:#FFD700;">Surcoinche</strong> — Redouble les points si vous êtes coinché et sûr de réussir.</li>
    <li style="margin-bottom:14px;padding-left:16px;border-left:3px solid #FFD700;"><strong style="color:#FFD700;">Capot</strong> — Annoncer que votre équipe va remporter tous les plis (250 points).</li>
    <li style="margin-bottom:14px;padding-left:16px;border-left:3px solid #FFD700;"><strong style="color:#FFD700;">Générale</strong> — Annoncer que vous allez remporter tous les plis (500 points).</li>
    <li style="margin-bottom:14px;padding-left:16px;border-left:3px solid #FFD700;"><strong style="color:#FFD700;">Tout Atout (TA)</strong> — Toutes les couleurs deviennent atout. Ordre : Valet (14), 9 (9), As (6), 10 (4), Roi (3), Dame (2), 8 (0), 7 (0).</li>
    <li style="margin-bottom:14px;padding-left:16px;border-left:3px solid #FFD700;"><strong style="color:#FFD700;">Sans Atout (SA)</strong> — Aucune couleur n'est atout. Ordre : As (19), 10 (10), Roi (4), Dame (3), Valet (2), 9 (0), 8 (0), 7 (0).</li>
  </ul>

  <!-- ─── DÉROULEMENT D'UN PLI ─── -->
  <h2 style="color:#FFD700;font-size:1.8em;border-bottom:2px solid #FFD700;padding-bottom:8px;margin-top:40px;">Déroulement d'un pli</h2>
  <p>Le joueur à la gauche du Dealer débute la partie.</p>
  <h3 style="color:#FFD700;margin-top:28px;">Règles obligatoires</h3>
  <ol>
    <li>Vous devez fournir la couleur demandée si vous l'avez.</li>
    <li>Si vous n'avez pas la couleur demandée :
      <ul style="margin-top:8px;">
        <li>Si votre partenaire est maître du pli : vous pouvez jouer ce que vous voulez (défausser).</li>
        <li>Si votre partenaire n'est pas maître : vous devez couper avec l'atout si possible.</li>
        <li>Si un atout a déjà été joué et que vous devez couper : vous devez surmonter si possible.</li>
      </ul>
    </li>
    <li>Si vous ne pouvez ni fournir ni couper, vous défaussez.</li>
    <li>Le joueur qui pose la carte la plus forte remporte le pli. Le gagnant du pli joue la carte suivante.</li>
  </ol>

  <!-- ─── COMPTAGE DES POINTS ─── -->
  <h2 style="color:#FFD700;font-size:1.8em;border-bottom:2px solid #FFD700;padding-bottom:8px;margin-top:40px;">Comptage des points</h2>
  <p>À la fin de la manche : le total des points dans le jeu est de <strong>162 points</strong> (152 + 10 de dix de der). L'équipe qui remporte le dernier pli gagne <strong>10 points bonus</strong> (dix de der).</p>

  <h3 style="color:#FFD700;margin-top:28px;">Si le contrat est réussi</h3>
  <ul>
    <li>L'équipe qui a pris marque le nombre de points annoncés.</li>
    <li>L'équipe adverse marque ses points réalisés.</li>
  </ul>

  <h3 style="color:#FFD700;margin-top:28px;">Si le contrat échoue</h3>
  <ul>
    <li>L'équipe qui a pris ne marque rien.</li>
    <li>L'équipe adverse marque <strong>162 points + le contrat annoncé</strong>.</li>
  </ul>

  <h3 style="color:#FFD700;margin-top:28px;">En cas de coinche / surcoinche</h3>
  <p>Les points du contrat sont multipliés par <strong>2</strong> (coinche) ou <strong>4</strong> (surcoinche).</p>

  <!-- ─── COINCHE À LA VOLÉE ─── -->
  <h2 style="color:#FFD700;font-size:1.8em;border-bottom:2px solid #FFD700;padding-bottom:8px;margin-top:40px;">Coinche à la volée</h2>
  <p>Dans cette version de la Coinche, la coinche à la volée est autorisée :</p>
  <ul>
    <li>N'importe quel joueur de l'équipe adverse peut coincher <strong>immédiatement</strong> après l'annonce d'un contrat.</li>
    <li>Il n'est pas nécessaire d'attendre son tour pour coincher.</li>
    <li>Le bouton « Coinche » apparaît pendant quelques secondes après chaque annonce adverse.</li>
    <li>Une fois qu'un joueur a coinché, seule l'équipe attaquante peut surcoincher.</li>
  </ul>
  <p style="color:#FFD700;"><strong>Cette règle ajoute du dynamisme aux enchères et permet de réagir rapidement à une annonce jugée trop ambitieuse !</strong></p>

  <!-- ─── BELOTE ET REBELOTE ─── -->
  <h2 style="color:#FFD700;font-size:1.8em;border-bottom:2px solid #FFD700;padding-bottom:8px;margin-top:40px;">Belote et Rebelote</h2>
  <p>Si vous possédez le Roi et la Dame d'atout, vous pouvez annoncer :</p>
  <ul>
    <li>« Belote » en jouant la première carte.</li>
    <li>« Rebelote » en jouant la seconde.</li>
  </ul>
  <p>Cela rapporte <strong>20 points bonus</strong> à votre équipe.</p>
  <p style="background:#1a1a2e;padding:14px 18px;border-radius:6px;border-left:4px solid #FFD700;color:#FFD700;margin:16px 0;"><strong>Important :</strong> Si l'équipe qui a pris le contrat possède la belote, ces 20 points comptent comme des points en moins à réaliser. Exemple : contrat de 100 points avec belote → il suffit de réaliser 80 points. Il n'y a pas de belote en TA et en SA.</p>

  <!-- ─── FIN DE PARTIE ─── -->
  <h2 style="color:#FFD700;font-size:1.8em;border-bottom:2px solid #FFD700;padding-bottom:8px;margin-top:40px;">Fin de partie</h2>
  <p>La partie se termine lorsqu'une équipe atteint <strong>1000 points</strong> ou plus. L'équipe avec le plus de points gagne la partie !</p>

  <!-- ─── DÉCONNEXION ─── -->
  <h2 style="color:#FFD700;font-size:1.8em;border-bottom:2px solid #FFD700;padding-bottom:8px;margin-top:40px;">Déconnexion et reconnexion</h2>

  <h3 style="color:#FFD700;margin-top:28px;">Déconnexion involontaire</h3>
  <ul>
    <li>Votre place est temporairement prise par un bot.</li>
    <li>Vous pouvez rejoindre la partie en cours automatiquement en vous reconnectant.</li>
    <li>Vos cartes et votre position sont conservées.</li>
    <li>Si vous ne revenez pas avant la fin de la partie, une défaite est comptabilisée.</li>
  </ul>

  <h3 style="color:#FFD700;margin-top:28px;">Abandon volontaire (bouton Quitter)</h3>
  <ul>
    <li>Vous abandonnez définitivement la partie.</li>
    <li>Votre place est remplacée par un bot pour le reste de la partie.</li>
    <li>Une défaite est comptabilisée immédiatement dans vos statistiques.</li>
    <li>Vous ne pourrez pas rejoindre cette partie.</li>
  </ul>

  <h3 style="color:#FFD700;margin-top:28px;">Impact sur les statistiques</h3>
  <ul>
    <li><strong>Victoire :</strong> comptabilisée uniquement si vous êtes présent à la fin quand votre équipe gagne.</li>
    <li><strong>Défaite par abandon :</strong> comptabilisée immédiatement lorsque vous cliquez sur « Quitter ».</li>
    <li><strong>Déconnexion sans retour :</strong> comptabilisée comme une défaite, même si votre équipe gagne grâce au bot.</li>
  </ul>
</div>
<!-- /wp:html -->
__END_B__;

$id_b = coinche_upsert_page( 'Règles du Jeu', 'regles-du-jeu', $content_b );
if ( is_wp_error( $id_b ) ) coinche_err( 'Règles du Jeu : ' . $id_b->get_error_message() );


// ============================================================
// C — PAGE CONTACT
// ============================================================
$content_c = <<<'__END_C__'
<!-- wp:html -->
<div style="max-width:680px;margin:0 auto;padding:80px 24px;font-family:'Segoe UI',Tahoma,sans-serif;color:#cccccc;text-align:center;line-height:1.7;">

  <h1 style="color:#FFD700;font-size:2.5em;margin-bottom:30px;">Contact</h1>
  <p style="font-size:1.1em;margin-bottom:30px;">Pour toute demande concernant Coinche de l'Espace, n'hésitez pas à nous contacter :</p>
  <p style="font-size:1.5em;margin-bottom:10px;">📧 <a href="mailto:contact@nebuludik.fr" style="color:#FFD700;text-decoration:none;font-weight:bold;">contact@nebuludik.fr</a></p>
  <p style="color:#777;font-style:italic;font-size:0.92em;">Nous vous répondrons dans les meilleurs délais.</p>
</div>
<!-- /wp:html -->
__END_C__;

$id_c = coinche_upsert_page( 'Contact', 'contact', $content_c );
if ( is_wp_error( $id_c ) ) coinche_err( 'Contact : ' . $id_c->get_error_message() );


// ============================================================
// D — PAGE POLITIQUE DE CONFIDENTIALITÉ
// ============================================================
$content_d = <<<'__END_D__'
<!-- wp:html -->
<div style="max-width:860px;margin:0 auto;padding:50px 24px;font-family:'Segoe UI',Tahoma,sans-serif;color:#cccccc;line-height:1.7;">

  <h1 style="color:#FFD700;text-align:center;font-size:2.5em;margin:0 0 10px 0;">Politique de Confidentialité</h1>
  <p style="text-align:center;color:#aaa;font-size:1.1em;margin-bottom:50px;">Coinche de l'Espace</p>
  <p style="text-align:center;color:#888;font-style:italic;margin-bottom:40px;">Dernière mise à jour : 17 février 2026</p>

  <!-- ─── 1. RESPONSABLE DU TRAITEMENT ─── -->
  <h2 style="color:#FFD700;font-size:1.8em;border-bottom:2px solid #FFD700;padding-bottom:8px;margin-top:40px;">1. Responsable du traitement</h2>
  <p>Le responsable du traitement des données est :</p>
  <p style="margin-left:20px;">
    <strong style="color:#FFD700;">Nebuludik</strong><br>
    Email : <a href="mailto:contact@nebuludik.fr" style="color:#FFD700;text-decoration:none;">contact@nebuludik.fr</a>
  </p>

  <!-- ─── 2. INTRODUCTION ─── -->
  <h2 style="color:#FFD700;font-size:1.8em;border-bottom:2px solid #FFD700;padding-bottom:8px;margin-top:40px;">2. Introduction</h2>
  <p>La protection de votre vie privée est importante pour nous. Cette politique explique quelles données sont collectées lorsque vous utilisez Coinche de l'Espace, pourquoi elles sont collectées et comment elles sont protégées.</p>
  <p>En utilisant l'application, vous acceptez les pratiques décrites ci-dessous.</p>

  <!-- ─── 3. DONNÉES COLLECTÉES ─── -->
  <h2 style="color:#FFD700;font-size:1.8em;border-bottom:2px solid #FFD700;padding-bottom:8px;margin-top:40px;">3. Données collectées</h2>
  <p>Nous collectons uniquement les données strictement nécessaires au fonctionnement du jeu :</p>
  <ul style="list-style:none;padding:0;margin:0;">
    <li style="margin-bottom:14px;padding-left:16px;border-left:3px solid #FFD700;">
      <strong style="color:#FFD700;">Adresse e-mail</strong> — création et gestion du compte
    </li>
    <li style="margin-bottom:14px;padding-left:16px;border-left:3px solid #FFD700;">
      <strong style="color:#FFD700;">Pseudonyme</strong> — identification en jeu
    </li>
    <li style="margin-bottom:14px;padding-left:16px;border-left:3px solid #FFD700;">
      <strong style="color:#FFD700;">Mot de passe chiffré</strong> — sécurité du compte
    </li>
    <li style="margin-bottom:14px;padding-left:16px;border-left:3px solid #FFD700;">
      <strong style="color:#FFD700;">Statistiques de jeu</strong> — scores, parties, classements
    </li>
  </ul>
  <p style="background:#1a1a2e;padding:14px 18px;border-radius:6px;border-left:4px solid #4caf50;color:#fff;margin:20px 0;">
    <strong>Nous ne collectons aucune donnée sensible</strong>, ni localisation, ni contacts, ni fichiers personnels.
  </p>

  <!-- ─── 4. FINALITÉS DU TRAITEMENT ─── -->
  <h2 style="color:#FFD700;font-size:1.8em;border-bottom:2px solid #FFD700;padding-bottom:8px;margin-top:40px;">4. Finalités du traitement</h2>
  <p>Les données sont utilisées uniquement pour :</p>
  <ul>
    <li>créer et gérer votre compte utilisateur</li>
    <li>permettre le fonctionnement du jeu en ligne</li>
    <li>afficher les scores et classements</li>
    <li>répondre aux demandes d'assistance</li>
    <li>sécuriser les comptes</li>
  </ul>
  <p style="color:#FFD700;"><strong>Aucune donnée n'est utilisée à des fins publicitaires ou commerciales.</strong></p>

  <!-- ─── 5. BASE LÉGALE ─── -->
  <h2 style="color:#FFD700;font-size:1.8em;border-bottom:2px solid #FFD700;padding-bottom:8px;margin-top:40px;">5. Base légale</h2>
  <p>Le traitement repose sur :</p>
  <ul>
    <li>l'exécution du service (fonctionnement du jeu)</li>
    <li>votre consentement lors de la création du compte</li>
  </ul>

  <!-- ─── 6. HÉBERGEMENT ET SÉCURITÉ ─── -->
  <h2 style="color:#FFD700;font-size:1.8em;border-bottom:2px solid #FFD700;padding-bottom:8px;margin-top:40px;">6. Hébergement et sécurité</h2>
  <ul>
    <li>Les données sont hébergées chez <strong style="color:#FFD700;">OVHcloud</strong> sur des serveurs situés en <strong style="color:#FFD700;">France</strong>.</li>
    <li>Les communications sont chiffrées via protocole <strong style="color:#FFD700;">SSL/TLS</strong>.</li>
    <li>Les mots de passe sont stockés sous forme hachée et sécurisée.</li>
    <li>Des mesures techniques sont mises en œuvre pour empêcher tout accès non autorisé.</li>
  </ul>

  <!-- ─── 7. PARTAGE DES DONNÉES ─── -->
  <h2 style="color:#FFD700;font-size:1.8em;border-bottom:2px solid #FFD700;padding-bottom:8px;margin-top:40px;">7. Partage des données</h2>
  <p>Nous ne vendons, louons ni partageons vos données personnelles avec des tiers.</p>
  <p>Les données sont utilisées <strong style="color:#FFD700;">exclusivement</strong> pour le fonctionnement de l'application.</p>

  <!-- ─── 8. DURÉE DE CONSERVATION ─── -->
  <h2 style="color:#FFD700;font-size:1.8em;border-bottom:2px solid #FFD700;padding-bottom:8px;margin-top:40px;">8. Durée de conservation</h2>
  <ul>
    <li>Les données sont conservées tant que votre compte est actif.</li>
    <li>En cas de suppression du compte, les données sont supprimées immédiatement.</li>
    <li>Les demandes de suppression par email sont traitées sous <strong style="color:#FFD700;">30 jours maximum</strong>.</li>
  </ul>

  <!-- ─── 9. DROITS DES UTILISATEURS (RGPD) ─── -->
  <h2 style="color:#FFD700;font-size:1.8em;border-bottom:2px solid #FFD700;padding-bottom:8px;margin-top:40px;">9. Droits des utilisateurs (RGPD)</h2>
  <p>Conformément au Règlement Général sur la Protection des Données, vous disposez des droits suivants :</p>
  <ul>
    <li>accès à vos données</li>
    <li>rectification</li>
    <li>suppression</li>
    <li>limitation du traitement</li>
    <li>opposition</li>
    <li>portabilité</li>
  </ul>
  <p style="background:#1a1a2e;padding:14px 18px;border-radius:6px;border-left:4px solid #FFD700;color:#FFD700;margin:20px 0;">
    <strong>Pour exercer ces droits :</strong><br>
    📩 <a href="mailto:contact@nebuludik.fr" style="color:#FFD700;text-decoration:none;">contact@nebuludik.fr</a>
  </p>

  <!-- ─── 10. SUPPRESSION DE COMPTE ─── -->
  <h2 style="color:#FFD700;font-size:1.8em;border-bottom:2px solid #FFD700;padding-bottom:8px;margin-top:40px;">10. Suppression de compte</h2>
  <p>Vous pouvez supprimer votre compte directement depuis l'application dans les paramètres.</p>

  <!-- ─── 11. ENFANTS ─── -->
  <h2 style="color:#FFD700;font-size:1.8em;border-bottom:2px solid #FFD700;padding-bottom:8px;margin-top:40px;">11. Enfants</h2>
  <p>L'application ne cible pas spécifiquement les enfants de moins de 13 ans et ne collecte pas sciemment de données les concernant.</p>

  <!-- ─── 12. MODIFICATIONS ─── -->
  <h2 style="color:#FFD700;font-size:1.8em;border-bottom:2px solid #FFD700;padding-bottom:8px;margin-top:40px;">12. Modifications</h2>
  <p>Cette politique peut être modifiée à tout moment. La version la plus récente est toujours disponible sur cette page.</p>

  <!-- ─── 13. CONTACT ─── -->
  <h2 style="color:#FFD700;font-size:1.8em;border-bottom:2px solid #FFD700;padding-bottom:8px;margin-top:40px;">13. Contact</h2>
  <p>Pour toute question concernant la confidentialité :</p>
  <p style="font-size:1.2em;margin:10px 0;">
    📧 <a href="mailto:contact@nebuludik.fr" style="color:#FFD700;text-decoration:none;font-weight:bold;">contact@nebuludik.fr</a>
  </p>
</div>
<!-- /wp:html -->
__END_D__;

$id_d = coinche_upsert_page( 'Politique de Confidentialité', 'politique-de-confidentialite', $content_d );
if ( is_wp_error( $id_d ) ) coinche_err( 'Politique de Confidentialité : ' . $id_d->get_error_message() );


// ============================================================
// PAGE D'ACCUEIL = Présentation
// ============================================================
if ( ! is_wp_error( $id_a ) ) {
    update_option( 'show_on_front', 'page' );
    update_option( 'page_on_front', $id_a );
    coinche_ok( "Page d'accueil définie → Présentation" );
}


// ============================================================
// MENU DE NAVIGATION (créé une seule fois)
// ============================================================
$menu_obj = wp_get_nav_menu_object( 'Menu Principal' );
if ( $menu_obj ) {
    coinche_log( 'Menu déjà existant — pas de modification.' );
} else {
    $menu_id = wp_create_nav_menu( 'Menu Principal' );
    if ( is_wp_error( $menu_id ) ) {
        coinche_err( 'Erreur création menu.' );
    } else {
        wp_add_nav_menu_item( $menu_id, [
            'menu-item-title'     => 'Présentation',
            'menu-item-status'    => 'publish',
            'menu-item-type'      => 'post_type',
            'menu-item-object'    => 'page',
            'menu-item-object-id' => $id_a,
        ] );
        wp_add_nav_menu_item( $menu_id, [
            'menu-item-title'     => 'Règles du Jeu',
            'menu-item-status'    => 'publish',
            'menu-item-type'      => 'post_type',
            'menu-item-object'    => 'page',
            'menu-item-object-id' => $id_b,
        ] );
        wp_add_nav_menu_item( $menu_id, [
            'menu-item-title'     => 'Politique de Confidentialité',
            'menu-item-status'    => 'publish',
            'menu-item-type'      => 'post_type',
            'menu-item-object'    => 'page',
            'menu-item-object-id' => $id_d,
        ] );
        wp_add_nav_menu_item( $menu_id, [
            'menu-item-title'     => 'Contact',
            'menu-item-status'    => 'publish',
            'menu-item-type'      => 'post_type',
            'menu-item-object'    => 'page',
            'menu-item-object-id' => $id_c,
        ] );

        $locations = get_theme_mod( 'nav_menu_locations', [] );
        $locations['primary'] = $menu_id;
        set_theme_mod( 'nav_menu_locations', $locations );
        coinche_ok( 'Menu créé et attaché.' );
    }
}


// ============================================================
// MU-PLUGIN : déployé manuellement via SFTP
// → website/mu-plugins/coinche-dark-theme.php
// ============================================================
coinche_log( 'Thème sombre : déployé séparément via SFTP (mu-plugins/).' );


// ============================================================
// RÉSUMÉ FINAL
// ============================================================
echo '<hr style="border-color:#222244;margin:30px 0;">';
echo '<h2>Permaliens créés</h2>';
if ( ! is_wp_error( $id_a ) ) coinche_log( 'Présentation                  : <a href="' . get_permalink( $id_a ) . '" target="_blank">' . get_permalink( $id_a ) . '</a>' );
if ( ! is_wp_error( $id_b ) ) coinche_log( 'Règles du Jeu                 : <a href="' . get_permalink( $id_b ) . '" target="_blank">' . get_permalink( $id_b ) . '</a>' );
if ( ! is_wp_error( $id_d ) ) coinche_log( 'Politique de Confidentialité  : <a href="' . get_permalink( $id_d ) . '" target="_blank">' . get_permalink( $id_d ) . '</a>' );
if ( ! is_wp_error( $id_c ) ) coinche_log( 'Contact                       : <a href="' . get_permalink( $id_c ) . '" target="_blank">' . get_permalink( $id_c ) . '</a>' );

echo '</div></body></html>';
?>
