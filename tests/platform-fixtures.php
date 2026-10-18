<?php
// Isolated test database only; never load production config or credentials.
declare(strict_types=1);
$db=new PDO('mysql:host=127.0.0.1;port=19336;dbname=cloud_test;charset=utf8mb4','root','',[PDO::ATTR_ERRMODE=>PDO::ERRMODE_EXCEPTION]);
if (($argv[1] ?? '')==='seed') {
    $db->exec("INSERT INTO platform_games(id,title,return_path) VALUES ('second-game','Second game','/games/second/') ON DUPLICATE KEY UPDATE active=1");
    $db->exec("INSERT INTO platform_products(game_id,id,title,stripe_price,amount,currency,active) VALUES ('paws','beach_afternoon','Beach adventure','price_fixture',500,'sek',1),('second-game','beach_afternoon','Other unlock','price_other',700,'sek',1) ON DUPLICATE KEY UPDATE active=1");
} elseif (($argv[1] ?? '')==='reset') {
    $q=$db->prepare('INSERT INTO platform_password_resets(user_id,token_hash,expires_at) VALUES (?,?,?) ON DUPLICATE KEY UPDATE token_hash=VALUES(token_hash),expires_at=VALUES(expires_at)');
    $q->execute([$argv[2],hash('sha256',$argv[3]),time()+900]);
} elseif (($argv[1] ?? '')==='purchase') {
    $q=$db->prepare("INSERT INTO platform_purchases(id,user_id,game_id,product_id,request_key,stripe_price,amount,currency,return_path,stripe_session) VALUES (?,?,?,'beach_afternoon',?,'price_fixture',500,'sek','/games/paws/',?)");
    $q->execute([$argv[2],$argv[3],$argv[4],$argv[2],$argv[5]]);
}
