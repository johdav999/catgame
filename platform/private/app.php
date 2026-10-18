<?php
declare(strict_types=1);

final class GamePlatform
{
    private PDO $db;
    public function __construct(private array $config, private ?Closure $checkoutTransport=null) {
        if (strlen($config['rate_key'] ?? '') < 32) throw new RuntimeException('Configuration required');
        if (!empty($config['stripe_secret']) && !preg_match(($config['stripe_live'] ?? false)?'/^(sk|rk)_live_/':'/^(sk|rk)_test_/',$config['stripe_secret'])) throw new RuntimeException('Stripe mode mismatch');
        $this->db = new PDO($config['dsn'], $config['db_user'], $config['db_password'], [
            PDO::ATTR_ERRMODE=>PDO::ERRMODE_EXCEPTION, PDO::ATTR_EMULATE_PREPARES=>false,
            PDO::ATTR_DEFAULT_FETCH_MODE=>PDO::FETCH_ASSOC]);
    }
    private function query(string $sql, array $args = []): PDOStatement {
        $s=$this->db->prepare($sql); $s->execute($args); return $s;
    }
    private function reply(array $data, int $status=200): never {
        http_response_code($status); echo json_encode($data, JSON_THROW_ON_ERROR); exit;
    }
    private function fail(string $message, int $status): never { $this->reply(['error'=>$message],$status); }
    private function body(int $max): string {
        if ((int)($_SERVER['CONTENT_LENGTH'] ?? 0)>$max) $this->fail('Request too large.',413);
        $raw=file_get_contents('php://input',false,null,0,$max+1);
        if ($raw===false || strlen($raw)>$max) $this->fail('Request too large.',413);
        return $raw;
    }
    private function json(int $max=8192): array {
        if (strtolower(trim(explode(';',$_SERVER['CONTENT_TYPE'] ?? '')[0]))!=='application/json')
            $this->fail('JSON content type required.',415);
        try {$data=json_decode($this->body($max),true,32,JSON_THROW_ON_ERROR);}
        catch (JsonException) {$this->fail('Invalid JSON.',400);}
        if (!is_array($data) || array_is_list($data)) $this->fail('JSON object required.',400);
        return $data;
    }
    private function rate(string $name, string $identity, int $limit, int $seconds): void {
        $now=time(); $bucket=hash_hmac('sha256',"$name:$identity:".intdiv($now,$seconds),$this->config['rate_key']);
        $this->query('INSERT INTO platform_rate_limits(bucket,hits,expires_at) VALUES (?,1,?) ON DUPLICATE KEY UPDATE hits=hits+1',[$bucket,$now+$seconds]);
        $hits=$this->query('SELECT hits FROM platform_rate_limits WHERE bucket=?',[$bucket])->fetchColumn();
        $this->query('DELETE FROM platform_rate_limits WHERE expires_at < ? LIMIT 100',[$now]);
        if ($hits>$limit) {header('Retry-After: '.$seconds);$this->fail('Too many requests. Try again later.',429);}
    }
    private function user(): array {
        $id=$_SESSION['user_id'] ?? null;
        if (!$id) $this->fail('Please sign in.',401);
        $user=$this->query('SELECT id,email,session_version FROM platform_users WHERE id=?',[$id])->fetch();
        if (!$user || (int)$user['session_version']!==($_SESSION['version'] ?? 0)) $this->fail('Please sign in.',401);
        return ['id'=>(string)$user['id'],'email'=>$user['email']];
    }
    private function game(string $id): array {
        $game=$this->query('SELECT * FROM platform_games WHERE id=? AND active=1',[$id])->fetch();
        if (!$game) $this->fail('Game not found.',404);
        return $game;
    }
    private function purchases(string $user, string $game): array {
        return $this->query('SELECT id,product_id,amount,currency,status,refunded_amount,disputed,created_at FROM platform_purchases WHERE user_id=? AND game_id=? ORDER BY created_at DESC LIMIT 100',[$user,$game])->fetchAll();
    }
    private function entitlements(string $user,string $game): array {
        return $this->query("SELECT DISTINCT product_id FROM platform_purchases WHERE user_id=? AND game_id=? AND status='paid' AND disputed=0 AND refunded_amount<amount",[$user,$game])->fetchAll(PDO::FETCH_COLUMN);
    }
    public function run(): never {
        $route=$_GET['route'] ?? ''; $method=$_SERVER['REQUEST_METHOD'];
        if (!is_string($route)) $this->fail('Invalid route.',400);
        if ($route==='/v1/stripe/webhook' && $method==='POST') $this->webhook();
        if (($this->config['secure_cookies'] ?? true) && ($_SERVER['HTTPS'] ?? '')!=='on')
            $this->fail('HTTPS required.',400);
        // All browser clients share this exact origin. No wildcard CORS or parent-domain cookie.
        $origin=$_SERVER['HTTP_ORIGIN'] ?? null;
        if ($origin!==null && $origin!==$this->config['origin']) $this->fail('Origin not allowed.',403);
        if (in_array($_SERVER['HTTP_SEC_FETCH_SITE'] ?? '', ['cross-site','same-site'],true)) $this->fail('Same origin required.',403);
        ini_set('session.use_strict_mode','1'); ini_set('session.use_only_cookies','1');
        session_name('game_platform');
        session_set_cookie_params(['lifetime'=>0,'path'=>'/','secure'=>$this->config['secure_cookies'] ?? true,'httponly'=>true,'samesite'=>'Lax']);
        session_start();
        if (isset($_SESSION['last_seen']) && (time()-$_SESSION['last_seen']>7200 || time()-$_SESSION['started']>86400)) {
            $_SESSION=[];session_regenerate_id(true);
        }
        $_SESSION['started'] ??= time(); $_SESSION['last_seen']=time();
        $_SESSION['csrf'] ??= bin2hex(random_bytes(32));
        if ($method!=='GET' && !hash_equals($_SESSION['csrf'],$_SERVER['HTTP_X_CSRF_TOKEN'] ?? '')) $this->fail('Refresh your session and try again.',403);
        if ($route==='/v1/session' && $method==='GET') {
            $this->reply(['user'=>isset($_SESSION['user_id'])?$this->user():null,'csrf'=>$_SESSION['csrf']]);
        }
        if ($route==='/v1/password/request' && $method==='POST') {
            $this->rate('reset-ip',$_SERVER['REMOTE_ADDR'] ?? '',10,3600);
            $b=$this->json();$email=strtolower(trim(is_string($b['email'] ?? null)?$b['email']:''));
            $this->rate('reset-email',$email,3,3600);
            $from=$this->config['mail_from'] ?? '';
            if (!filter_var($from,FILTER_VALIDATE_EMAIL) || preg_match('/[\r\n]/',$from)) $this->fail('Account recovery is not configured yet.',503);
            $u=$this->query('SELECT id FROM platform_users WHERE email=?',[$email])->fetch();
            if ($u) {
                $token=bin2hex(random_bytes(32));
                $this->query('INSERT INTO platform_password_resets(user_id,token_hash,expires_at) VALUES (?,?,?) ON DUPLICATE KEY UPDATE token_hash=VALUES(token_hash),expires_at=VALUES(expires_at)',[$u['id'],hash('sha256',$token),time()+900]);
                $url=$this->config['origin'].'/account-reset.html#'.$token;
                // The token is never returned to the requesting browser or written to a log.
                if (!mail($email,'Reset your games account password',"Use this link within 15 minutes:\n".$url."\nIf you did not request this, ignore this email.",['From'=>$from])) {
                    $this->query('DELETE FROM platform_password_resets WHERE user_id=?',[$u['id']]);
                }
            }
            $this->reply(['message'=>'If this account exists, a reset link has been sent.']);
        }
        if ($route==='/v1/password/reset' && $method==='POST') {
            $this->rate('reset-confirm',$_SERVER['REMOTE_ADDR'] ?? '',20,3600);$b=$this->json();
            $token=$b['token'] ?? '';$password=$b['password'] ?? '';
            if (!is_string($token)||!preg_match('/^[a-f0-9]{64}$/D',$token)||!is_string($password)||strlen($password)<12||strlen($password)>72) $this->fail('Invalid reset link or password.',400);
            $this->db->beginTransaction();
            $reset=$this->query('SELECT * FROM platform_password_resets WHERE token_hash=? FOR UPDATE',[hash('sha256',$token)])->fetch();
            if (!$reset || (int)$reset['expires_at']<time()) {$this->db->rollBack();$this->fail('This link has expired or was already used.',400);}
            $this->query('UPDATE platform_users SET password_hash=?,session_version=session_version+1 WHERE id=?',[password_hash($password,PASSWORD_DEFAULT),$reset['user_id']]);
            $this->query('DELETE FROM platform_password_resets WHERE user_id=?',[$reset['user_id']]);$this->db->commit();
            $_SESSION=[];session_regenerate_id(true);$_SESSION['csrf']=bin2hex(random_bytes(32));
            $this->reply(['message'=>'Password changed. Sign in again in your game.','csrf'=>$_SESSION['csrf']]);
        }
        if (in_array($route,['/v1/register','/v1/login'],true) && $method==='POST') {
            $this->rate('auth-ip',$_SERVER['REMOTE_ADDR'] ?? '',30,900);
            $b=$this->json();$email=strtolower(trim(is_string($b['email'] ?? null)?$b['email']:''));$password=$b['password'] ?? '';
            if (strlen($email)>254 || !filter_var($email,FILTER_VALIDATE_EMAIL) || !is_string($password) || strlen($password)<12 || strlen($password)>72)
                $this->fail('Use a valid email and a password of 12–72 bytes.',400);
            $this->rate('auth-email',$email,15,900);
            if ($route==='/v1/register') {
                if (!($this->config['registration_open'] ?? false)) $this->fail('Registration is closed.',403);
                try {$this->query('INSERT INTO platform_users(email,password_hash) VALUES (?,?)',[$email,password_hash($password,PASSWORD_DEFAULT)]);}
                catch (PDOException $e) {if ($e->getCode()==='23000') $this->fail('Unable to create this account. Try signing in.',409);throw $e;}
                $id=$this->db->lastInsertId();
            } else {
                $u=$this->query('SELECT * FROM platform_users WHERE email=?',[$email])->fetch();
                $hash=$u['password_hash'] ?? '$2y$10$92IXUNpkjO0rOQ5byMi.Ye4oKoEa3Ro9llC/.og/at2uheWG/igi.';
                if (!password_verify($password,$hash) || !$u) $this->fail('Email or password is incorrect.',401);
                $id=$u['id'];
                if (password_needs_rehash($hash,PASSWORD_DEFAULT)) $this->query('UPDATE platform_users SET password_hash=? WHERE id=?',[password_hash($password,PASSWORD_DEFAULT),$id]);
            }
            $version=(int)$this->query('SELECT session_version FROM platform_users WHERE id=?',[$id])->fetchColumn();
            session_regenerate_id(true);$_SESSION=['user_id'=>$id,'version'=>$version,'csrf'=>bin2hex(random_bytes(32)),'started'=>time(),'last_seen'=>time()];
            $this->reply(['user'=>$this->user(),'csrf'=>$_SESSION['csrf']],$route==='/v1/register'?201:200);
        }
        if ($route==='/v1/logout' && $method==='POST') {
            $_SESSION=[];session_regenerate_id(true);$_SESSION['csrf']=bin2hex(random_bytes(32));
            $this->reply(['user'=>null,'csrf'=>$_SESSION['csrf']]);
        }
        $u=$this->user();
        if ($route==='/v1/purchases' && $method==='GET') $this->reply(['purchases'=>$this->query('SELECT id,game_id,product_id,amount,currency,status,refunded_amount,disputed,created_at FROM platform_purchases WHERE user_id=? ORDER BY created_at DESC LIMIT 100',[$u['id']])->fetchAll()]);
        if (!preg_match('~^/v1/games/([a-z0-9][a-z0-9_-]{0,63})/(save|products|purchases|entitlements|checkout)$~D',$route,$m)) $this->fail('Endpoint not found.',404);
        $game=$this->game($m[1]);$action=$m[2];$uid=$u['id'];$gid=$game['id'];
        if ($action==='save' && $method==='GET') {
            $s=$this->query('SELECT revision,format_version,payload,updated_at FROM platform_saves WHERE user_id=? AND game_id=?',[$uid,$gid])->fetch();
            $this->reply(['save'=>$s?['revision'=>(int)$s['revision'],'formatVersion'=>(int)$s['format_version'],'payload'=>base64_encode($s['payload']),'updatedAt'=>$s['updated_at']]:null]);
        }
        if ($action==='save' && $method==='PUT') {
            $this->rate('save',$uid,120,60);
            $b=$this->json((int)ceil($game['max_save_bytes']*4/3)+1024);
            if (!is_int($b['expectedRevision'] ?? null) || $b['expectedRevision']<0 || !is_int($b['formatVersion'] ?? null) || $b['formatVersion']<1 || !is_string($b['payload'] ?? null)) $this->fail('Invalid save envelope.',400);
            $bytes=base64_decode($b['payload'],true);
            if ($bytes===false || strlen($bytes)<1 || strlen($bytes)>$game['max_save_bytes'] || strlen($bytes)>16777215) $this->fail('Invalid save size or encoding.',400);
            $this->db->beginTransaction();
            // Serialize even first inserts; row locks alone cannot lock a missing save reliably.
            $this->query('SELECT id FROM platform_users WHERE id=? FOR UPDATE',[$uid]);
            $existing=$this->query('SELECT revision,payload,format_version FROM platform_saves WHERE user_id=? AND game_id=?',[$uid,$gid])->fetch();
            $revision=$existing?(int)$existing['revision']:0;
            // A lost response may be safely retried with the same body.
            if ($existing && $revision===$b['expectedRevision']+1 && $existing['payload']===$bytes && (int)$existing['format_version']===$b['formatVersion']) {
                $this->db->commit();$this->reply(['revision'=>$revision]);
            }
            if ($revision!==$b['expectedRevision']) {$this->db->rollBack();$this->reply(['error'=>'Cloud progress changed on another device. Load it before saving again.','revision'=>$revision],409);}
            $this->query('INSERT INTO platform_saves(user_id,game_id,revision,format_version,payload) VALUES (?,?,?,?,?) ON DUPLICATE KEY UPDATE revision=VALUES(revision),format_version=VALUES(format_version),payload=VALUES(payload)',[$uid,$gid,$revision+1,$b['formatVersion'],$bytes]);
            $this->db->commit();$this->reply(['revision'=>$revision+1]);
        }
        if ($action==='products' && $method==='GET') {
            $this->reply(['products'=>$this->query('SELECT id,title,amount,currency FROM platform_products WHERE game_id=? AND active=1',[$gid])->fetchAll()]);
        }
        if ($action==='purchases' && $method==='GET') $this->reply(['purchases'=>$this->purchases($uid,$gid)]);
        if ($action==='entitlements' && $method==='GET') $this->reply(['entitlements'=>$this->entitlements($uid,$gid)]);
        if ($action==='checkout' && $method==='POST') $this->checkout($u,$game);
        $this->fail('Method not allowed.',405);
    }
    private function checkout(array $user,array $game): never {
        if (empty($this->config['stripe_secret'])) $this->fail('Purchases are not configured yet.',503);
        $this->rate('checkout',$user['id'],20,3600);
        $b=$this->json();$pid=$b['productId'] ?? '';$key=$b['requestKey'] ?? '';
        if (!is_string($pid) || !is_string($key) || !preg_match('/^[a-zA-Z0-9-]{16,64}$/D',$key)) $this->fail('Invalid purchase request.',400);
        $product=$this->query('SELECT * FROM platform_products WHERE game_id=? AND id=? AND active=1',[$game['id'],$pid])->fetch();
        if (!$product) $this->fail('Product not found.',404);
        $this->db->beginTransaction();$this->query('SELECT id FROM platform_users WHERE id=? FOR UPDATE',[$user['id']]);
        $order=$this->query('SELECT * FROM platform_purchases WHERE user_id=? AND request_key=?',[$user['id'],$key])->fetch();
        if ($order && ($order['game_id']!==$game['id'] || $order['product_id']!==$pid)) {$this->db->rollBack();$this->fail('Request key already used.',409);}
        if ($order && $order['status']!=='pending') {$this->db->rollBack();$this->reply(['error'=>'This checkout is closed. Refresh purchases before trying again.','code'=>'checkout_closed'],409);}
        if (!$order) $order=$this->query("SELECT * FROM platform_purchases WHERE user_id=? AND game_id=? AND product_id=? AND status='pending' ORDER BY created_at LIMIT 1",[$user['id'],$game['id'],$pid])->fetch();
        if (!$order) {
            if (in_array($pid,$this->entitlements($user['id'],$game['id']),true)) {$this->db->rollBack();$this->fail('You already own this item.',409);}
            $id=bin2hex(random_bytes(16));
            $this->query('INSERT INTO platform_purchases(id,user_id,game_id,product_id,request_key,stripe_price,amount,currency,return_path) VALUES (?,?,?,?,?,?,?,?,?)',[$id,$user['id'],$game['id'],$pid,$key,$product['stripe_price'],$product['amount'],$product['currency'],$game['return_path']]);
            $order=$this->query('SELECT * FROM platform_purchases WHERE id=?',[$id])->fetch();
        }
        $this->db->commit();
        if ($order['checkout_url']) $this->reply(['url'=>$order['checkout_url'],'purchaseId'=>$order['id']]);
        // Never recreate an unknown paid outcome after Stripe's idempotency retention window.
        if (time()-strtotime($order['created_at'].' UTC')>82800) $this->fail('This checkout needs support review before retrying.',409);
        $path=$order['return_path'];
        if (!str_starts_with($path,'/') || str_starts_with($path,'//') || str_contains($path,'?') || str_contains($path,'#')) throw new RuntimeException('Invalid return path');
        $return=$this->config['origin'].$path;
        $params=['mode'=>'payment','line_items'=>[['price'=>$order['stripe_price'],'quantity'=>1]],
            'success_url'=>$return.'?checkout=complete','cancel_url'=>$return.'?checkout=cancelled',
            'client_reference_id'=>$order['id'],'metadata'=>['purchase_id'=>$order['id']],
            'payment_intent_data'=>['metadata'=>['purchase_id'=>$order['id']]]];
        $session=$this->checkoutTransport ? ($this->checkoutTransport)($params,$order['id']) : $this->stripeCheckout($params,$order['id']);
        if (!is_string($session['id'] ?? null) || !is_string($session['url'] ?? null) || !str_starts_with($session['url'],'https://checkout.stripe.com/') || ($session['livemode'] ?? null)!==($this->config['stripe_live'] ?? false)) throw new RuntimeException('Invalid checkout');
        $this->query('UPDATE platform_purchases SET stripe_session=?,checkout_url=? WHERE id=?',[$session['id'],$session['url'],$order['id']]);
        $this->reply(['url'=>$session['url'],'purchaseId'=>$order['id']]);
    }
    private function stripeCheckout(array $params,string $orderId): array {
        $ch=curl_init('https://api.stripe.com/v1/checkout/sessions');
        curl_setopt_array($ch,[CURLOPT_POST=>true,CURLOPT_POSTFIELDS=>http_build_query($params),CURLOPT_RETURNTRANSFER=>true,
            CURLOPT_CONNECTTIMEOUT=>10,CURLOPT_TIMEOUT=>25,CURLOPT_HTTPHEADER=>[
                'Authorization: Bearer '.$this->config['stripe_secret'],'Idempotency-Key: game-platform-'.$orderId,
                'Stripe-Version: 2024-06-20','Content-Type: application/x-www-form-urlencoded']]);
        $raw=curl_exec($ch);$status=curl_getinfo($ch,CURLINFO_RESPONSE_CODE);curl_close($ch);
        if ($raw===false || $status!==200) $this->fail('Checkout could not be confirmed. Retry this same purchase request.',502);
        return json_decode($raw,true,32,JSON_THROW_ON_ERROR);
    }
    private function webhook(): never {
        $secret=$this->config['stripe_webhook_secret'] ?? '';
        if (!$secret) $this->fail('Payments unavailable.',503);
        $raw=$this->body(262144);$header=$_SERVER['HTTP_STRIPE_SIGNATURE'] ?? '';
        $parts=[];foreach(explode(',',$header) as $pair) {$kv=explode('=',$pair,2);if(count($kv)===2)$parts[trim($kv[0])][]=$kv[1];}
        $timestamp=$parts['t'][0] ?? '';
        if (!ctype_digit($timestamp) || abs(time()-(int)$timestamp)>300) $this->fail('Invalid signature.',400);
        $expected=hash_hmac('sha256',$timestamp.'.'.$raw,$secret);$ok=false;
        foreach($parts['v1'] ?? [] as $signature) $ok=hash_equals($expected,$signature)||$ok;
        if (!$ok) $this->fail('Invalid signature.',400);
        try {$event=json_decode($raw,true,32,JSON_THROW_ON_ERROR);}catch(JsonException){$this->fail('Invalid event.',400);}
        if (!is_string($event['id'] ?? null) || !is_string($event['type'] ?? null) || !is_bool($event['livemode'] ?? null) || $event['livemode']!==($this->config['stripe_live'] ?? false)) $this->fail('Invalid event mode.',400);
        $type=$event['type'];$object=$event['data']['object'] ?? [];
        $supported=['checkout.session.completed','checkout.session.async_payment_succeeded','checkout.session.async_payment_failed','checkout.session.expired','charge.refunded','charge.dispute.created','charge.dispute.closed'];
        if (!in_array($type,$supported,true)) $this->reply(['received'=>true]);
        $this->db->beginTransaction();
        // Serial event receipt plus unique IDs makes duplicate concurrent deliveries harmless.
        $insert=$this->query('INSERT IGNORE INTO platform_events(id,event_type) VALUES (?,?)',[$event['id'],$type]);
        if (!$insert->rowCount()) {$this->db->commit();$this->reply(['received'=>true]);}
        $isSession=str_starts_with($type,'checkout.');
        if ($isSession) {
            $purchase=$object['metadata']['purchase_id'] ?? '';
            $order=$this->query('SELECT * FROM platform_purchases WHERE id=? FOR UPDATE',[$purchase])->fetch();
        } else {
            $order=$this->query('SELECT * FROM platform_purchases WHERE payment_intent=? FOR UPDATE',[$object['payment_intent'] ?? ''])->fetch();
        }
        // Unrelated Stripe activity belongs to other applications on this account.
        if (!$order) {$this->db->rollBack();$this->fail('Purchase not ready for this event. Retry delivery.',409);}
        if ($isSession) {
            if (($object['mode'] ?? '')!=='payment' || ($object['client_reference_id'] ?? '')!==$order['id'] ||
                ($order['stripe_session'] && ($object['id'] ?? '')!==$order['stripe_session']) ||
                ($object['amount_total'] ?? null)!==(int)$order['amount'] || ($object['currency'] ?? '')!==$order['currency']) {
                $this->db->rollBack();$this->fail('Purchase details do not match.',400);
            }
            if (in_array($type,['checkout.session.completed','checkout.session.async_payment_succeeded'],true) && ($object['payment_status'] ?? '')==='paid') {
                if (!is_string($object['payment_intent'] ?? null)) {$this->db->rollBack();$this->fail('Payment identity missing.',400);}
                $this->query("UPDATE platform_purchases SET status=IF(status='refunded','refunded','paid'),stripe_session=?,payment_intent=? WHERE id=?",[$object['id'],$object['payment_intent'],$order['id']]);
            } elseif (in_array($type,['checkout.session.async_payment_failed','checkout.session.expired'],true) && $order['status']==='pending') {
                $this->query("UPDATE platform_purchases SET status='cancelled' WHERE id=?",[$order['id']]);
            }
        } elseif ($type==='charge.refunded') {
            if (($object['currency'] ?? '')!==$order['currency'] || ($object['amount'] ?? null)!==(int)$order['amount'] || !is_int($object['amount_refunded'] ?? null)) {$this->db->rollBack();$this->fail('Refund details do not match.',400);}
            $amount=max((int)$order['refunded_amount'],$object['amount_refunded']);
            $this->query("UPDATE platform_purchases SET refunded_amount=?,status=IF(? >= amount,'refunded',status) WHERE id=?",[$amount,$amount,$order['id']]);
        } else {
            // Re-enabling after a dispute requires operator review. Out-of-order
            // dispute events must never accidentally restore ownership.
            $this->query('UPDATE platform_purchases SET disputed=1 WHERE id=?',[$order['id']]);
        }
        $this->db->commit();$this->reply(['received'=>true]);
    }
}
