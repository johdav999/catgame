<?php
// Local acceptance router. Never upload this file to the host.
$path=parse_url($_SERVER['REQUEST_URI'],PHP_URL_PATH);
if ($path==='/fixture-api.php') {
    // This injectable gateway exists only in the test router, outside public_html.
    header('Content-Type: application/json');
    $private=getenv('GAME_PLATFORM_PRIVATE');$config=require $private.'/config.php';
    $config['stripe_secret']='sk_test_fixture_not_a_real_secret';
    require dirname(__DIR__).'/platform/private/app.php';
    (new GamePlatform($config,static function(array $params,string $id): array {
        if ($params['mode']!=='payment'||$params['line_items']!==[['price'=>'price_fixture','quantity'=>1]]||
            $params['metadata']['purchase_id']!==$id||$params['payment_intent_data']['metadata']['purchase_id']!==$id||
            $params['success_url']!=='http://127.0.0.1:19337/games/paws/?checkout=complete') throw new RuntimeException('Incorrect checkout parameters');
        return ['id'=>'cs_gateway_'.$id,'url'=>'https://checkout.stripe.com/c/pay/fixture_'.$id,'livemode'=>false];
    }))->run();
}
if ($path==='/api/index.php') {require dirname(__DIR__).'/platform/public/api/index.php';return true;}
if ($path==='/account-reset.html' || $path==='/account-reset.js') $file=dirname(__DIR__).'/platform/public'.$path;
elseif (str_starts_with($path,'/games/paws/')) {
    $root=realpath(dirname(__DIR__).'/build/web-pinned-release/site');
    $relative=substr($path,strlen('/games/paws/'))?:'index.html';$file=realpath($root.'/'.$relative);
    if (!$file || !str_starts_with($file,$root.DIRECTORY_SEPARATOR)) {http_response_code(404);return true;}
} else {http_response_code(404);return true;}
$types=['html'=>'text/html','js'=>'text/javascript','css'=>'text/css','wasm'=>'application/wasm','json'=>'application/json','svg'=>'image/svg+xml','png'=>'image/png'];
header('Content-Type: '.($types[pathinfo($file,PATHINFO_EXTENSION)] ?? 'application/octet-stream'));
header('Cache-Control: no-store');readfile($file);return true;
