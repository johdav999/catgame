<?php
// Copy to config.php OUTSIDE the web root. Never commit credentials.
return [
    'dsn' => 'mysql:host=YOUR_LOOPIA_DB_HOST;dbname=YOUR_DATABASE;charset=utf8mb4',
    'db_user' => 'YOUR_DATABASE_USER',
    'db_password' => '',
    'origin' => 'https://games.example.com',
    'secure_cookies' => true,
    'rate_key' => '', // Generate: bin2hex(random_bytes(32))
    'stripe_secret' => '',
    'stripe_webhook_secret' => '',
    'stripe_live' => false,
    'registration_open' => true,
    'mail_from' => 'accounts@games.example.com', // Domain mailbox configured at Loopia
];
