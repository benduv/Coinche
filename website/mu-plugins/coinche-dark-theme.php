<?php
/**
 * Must-Use Plugin : Coinche de l'Espace — Thème sombre
 * Se charge automatiquement, pas besoin d'activer.
 */
add_action( 'wp_head', function() {
    echo '
    <style id="coinche-dark-theme">
        /* ── Base + fond étoilé ── */
        body {
            background-color: #0a0a1a !important;
            background-image: linear-gradient(rgba(10,10,26,0.55), rgba(10,10,26,0.55)),
                              url("/wp-content/uploads/cielEtoile.jpg") !important;
            background-size: cover !important;
            background-attachment: fixed !important;
            background-position: center !important;
            color: #cccccc;
        }
        /* ── En-tête ── */
        .site-header, header, .entry-header {
            background-color: rgba(10, 10, 26, 0.85) !important;
            border-bottom: 1px solid #222244;
        }
        /* ── Navigation ── */
        nav a, .nav-link, .nav-links a, .menu-link,
        .primary-nav a, .primary-menu a {
            color: #FFD700 !important;
        }
        nav a:hover { color: #ffe54d !important; }
        /* ── Liens ── */
        a { color: #FFD700 !important; }
        a:hover { color: #ffe54d !important; }
        /* ── Titres ── */
        h1, h2, h3, h4, h5, h6 { color: #FFD700; }
        /* ── Zone contenu semi-opaque (bande centrale uniquement) ── */
        [style*="max-width:860px"], [style*="max-width:680px"] {
            background-color: rgba(10, 10, 26, 0.75) !important;
            border-radius: 10px;
        }
        /* ── Tableaux ── */
        table { background-color: #111133; color: #cccccc; border-collapse: collapse; }
        table th { background-color: #1a1a3a; color: #FFD700; }
        table td, table th { border: 1px solid #333366; padding: 8px 12px; }
        /* ── Pied de page ── */
        footer, .site-footer {
            background-color: rgba(5, 5, 16, 0.85) !important;
            border-top: 1px solid #222244;
            color: #888;
        }
        footer a { color: #FFD700 !important; }
        /* ── Écraser les blancs du thème ── */
        .has-white-background-color,
        [style*="background:#fff"], [style*="background: #fff"],
        [style*="background-color:#fff"], [style*="background-color: #fff"],
        [style*="background:#ffffff"], [style*="background-color:#ffffff"] {
            background-color: transparent !important;
        }
        /* ── Scrollbar ── */
        ::-webkit-scrollbar { background: #0a0a1a; }
        ::-webkit-scrollbar-track { background: #111133; }
        ::-webkit-scrollbar-thumb { background: #333366; border-radius: 4px; }
    </style>';
} );
