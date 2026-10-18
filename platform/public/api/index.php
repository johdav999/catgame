<?php
declare(strict_types=1);
// Public entry point; application/configuration must live outside public_html.
ini_set('display_errors', '0');
header('Content-Type: application/json; charset=utf-8');
header('Cache-Control: no-store');
header('X-Content-Type-Options: nosniff');
header('Referrer-Policy: no-referrer');
try {
    $private = getenv('GAME_PLATFORM_PRIVATE') ?: dirname(__DIR__, 2) . '/private';
    $config = require $private . '/config.php';
    require $private . '/app.php';
    (new GamePlatform($config))->run();
} catch (Throwable $error) {
    // No SQL, credentials, provider response bodies or personal data in output/logs.
    http_response_code(503);
    echo json_encode(['error'=>'Service temporarily unavailable. Please try again.']);
}
