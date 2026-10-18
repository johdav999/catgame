# Shared games platform on Loopia

This repository now includes a PHP/MariaDB account, save and purchase backend in
`platform/`. Paws is the first client. Future games reuse the backend and accounts;
they receive their own registered game ID, products and saves. The API is HTTP
request/response, with no Node.js process, daemon, Composer or engine changes.

## Deployment layout

Use one HTTPS origin for all games, for example `https://games.example.com`:

```text
domain-directory/
  private/                     NEVER inside public_html
    app.php
    config.php                 created privately from config.example.php
  public_html/
    .htaccess
    api/index.php
    account-reset.html
    account-reset.js
    games/paws/                entire generated Paws site
    games/another-game/        another game's static site
```

`public_html` represents the web root shown in your Loopia account. The API loads
`../../private` relative to its own directory, not the current working directory.
If Loopia's filesystem layout differs, set the server-side environment variable
`GAME_PLATFORM_PRIVATE` to the absolute private folder or adjust the entry point's
private-folder locator. Confirm the private folder is outside every domain's web
root. Never upload database exports, config examples containing real secrets,
tests or the repository itself into the web root.

1. Select Loopia's PHP hosting and a supported PHP version, at least PHP 8.2,
   with PDO MySQL and cURL. Configure HTTPS. Production cookies require HTTPS;
   the application deliberately does not trust arbitrary forwarded headers.
2. Create a MariaDB database and user in Loopia Kundzon. Import `platform/schema.sql`
   once through phpMyAdmin. It creates only `platform_*` tables, uses InnoDB and
   registers `paws`. Use a dedicated database rather than importing over existing
   tables. Subsequent releases need deliberate migrations; this is initial schema 1.
3. Copy `platform/private/app.php` and a privately filled-out `config.php` to the
   private folder. Set the MariaDB host/database/user/password, exact origin
   (no trailing slash), and a random `rate_key` of at least 32 characters. Set
   `mail_from` to a domain mailbox authorized to send through your Loopia service.
   Restrict file permissions. Do not send or commit these secrets.
4. Upload `platform/public/` to the web root and the generated game `site/` to
   `/games/paws/`. Paws defaults to `/api/index.php`; its `game-api` meta element
   can change that path if needed. Set `platform_games.return_path` to the actual
   directory path, beginning and ending with `/`, without query strings/fragments.
5. Check Loopia's PHP memory/request-body limits against your save quota. Paws
   saves are base64 in JSON; a 16 MB binary approaches 22 MB over HTTP. The SQL
   MEDIUMBLOB hard limit is 16,777,215 bytes. Small future games default to 1 MB.
   Configure enough PHP memory (256 MB recommended for the maximum Paws save).
6. Verify `.wasm` receives `application/wasm`, scripts and assets load correctly,
   HTTPS cookies have Secure/HttpOnly/SameSite=Lax, and API responses use no-store.
   The included `.htaccess` supplies MIME and basic headers where supported.
   Test compression/caching and any host-specific directives on Loopia itself.
7. Verify registration, two-device save conflicts and recovery email delivery in
   staging. Set `registration_open=false` when registrations should be closed.
   Keep your own protected database backups and test restoration.

Build a deployment folder without secrets:

```powershell
./scripts/build.ps1 -Pinned -Configuration Release -EmsdkPath C:/path/to/emsdk
python scripts/package-loopia.py --site build/web-pinned-release/site
```

This produces `build/loopia-platform/` with public and private folders, the initial
schema and this guide. It deliberately includes only `config.example.php`, not
`config.php`. Configure the private credentials separately after upload. An
existing output folder is not overwritten. The package is a deployment candidate,
not evidence of a working Loopia deployment or live payment acceptance.

## Stripe setup

Start with Stripe test mode. Put the test secret and endpoint signing secret in
private config; keep `stripe_live=false`. Key prefixes and webhook livemode must
agree with that setting. No Stripe secrets enter JavaScript, the game save or a
public config file. Checkout calls pin Stripe API version `2024-06-20`.

Create one-time Stripe Prices for each durable game unlock. Insert corresponding
rows in `platform_products` with the game ID, stable product ID, title, exact
Stripe Price ID, positive amount in minor units and lower-case ISO currency.
Activate only reviewed products. Prices, currency, quantity and return URLs come
from the server; the browser supplies only product ID and an idempotency key.
Do not enable discounts, automatic tax or adjustable quantities without extending
the stored amount validation and tests. The initial implementation expects an
exact paid total and supports one-time durable unlocks. Subscriptions and
consumable currency require separate fulfillment/ledger semantics.

No products or prices are enabled in the shipped schema. To connect the current
Paws beach adventure, register a product with ID `beach_afternoon`. Paws applies
that server entitlement transiently and clears it on sign-out or failed startup
verification. The beach content remains a development greybox with its own open
acceptance gates; adding payment infrastructure does not approve it for sale.
The `?devBeach=1` development bypass is restricted to localhost; hosted origins
require the account entitlement. Static browser
code/assets can always be modified or copied by the player; this is account
ownership tracking, not DRM or an authoritative multiplayer economy.

Register this Stripe webhook URL:

```text
https://games.example.com/api/index.php?route=/v1/stripe/webhook
```

Subscribe to `checkout.session.completed`,
`checkout.session.async_payment_succeeded`,
`checkout.session.async_payment_failed`, `checkout.session.expired`,
`charge.refunded`, `charge.dispute.created` and `charge.dispute.closed`.
Use snapshot events compatible with the pinned API version.

The endpoint checks the signature over exact request bytes, a five-minute clock
tolerance, test/live mode, event ID, local order/session identity, amount and
currency. An unpaid completion never grants ownership. Receipt and fulfillment
commit in one transaction; retries cannot grant twice. The checkout success URL
never marks an order paid. Refresh purchases in-game after payment; refresh also
happens when opening the account panel and starting the game on a browser that
previously used an account. A local preference flag avoids probing a nonexistent
API during ordinary static/offline development; it contains no ownership data.

Each checkout has a persisted random ID and Stripe idempotency key. Reopening
an unresolved purchase reuses its session. An unknown outcome is not recreated
after the provider's idempotency retention window; it requires operator review.
Cancelled/expired checkouts require refreshing and starting a new request. A full
refund revokes ownership; a partial refund retains it. Late payment events cannot
undo a full refund. Disputes block ownership and require operator review before
re-enabling, even after a won dispute. Resolve unknown orders through Stripe's
dashboard, then replay the original signed event through Stripe's delivery tools.
Do not fabricate receipts or edit a local game save to grant purchases.

On an account shared with other applications, webhook events without a matching
platform purchase return 409 and grant nothing. Prefer a dedicated event
destination for these games; investigate unrelated deliveries rather than
treating them as missing customer purchases. Refunds arriving before their paid
event retry until the payment identity is recorded.

Before live sales, test a real Stripe test checkout, signed delivery, return to
the game, repeat delivery, delayed payment, refund, recovery email and a second
game. Then deliberately configure live keys/signing secret, `stripe_live=true`
and live Price IDs together. No live account was contacted during local tests.

## API contract

Use `GET /api/index.php?route=/v1/session` to obtain `{user,csrf}`. Send the cookie
with subsequent calls and `X-CSRF-Token` on every browser mutation, including
registration/login. A successful login rotates the session and CSRF token.
Sessions expire after two idle hours or 24 hours absolute. Password reset revokes
all earlier sessions through a user session version. Passwords use PHP's current
password hashing API. Reset tokens are hashed, single use, valid for 15 minutes
and delivered by email; reset URLs carry the token in the fragment rather than
the request URL. Email ownership is not asserted merely by registering.

| Route after `/v1` | Method | Purpose |
| --- | --- | --- |
| `/session` | GET | Account and CSRF token |
| `/register`, `/login` | POST | `{email,password}` |
| `/logout` | POST | End this session |
| `/password/request` | POST | `{email}`; generic recovery response |
| `/password/reset` | POST | `{token,password}`; revoke all sessions |
| `/purchases` | GET | Last 100 purchases across the account's games |
| `/games/{id}/save` | GET | `{save:null}` or revision/formatVersion/payload/updatedAt |
| `/games/{id}/save` | PUT | `{expectedRevision,formatVersion,payload}`; base64 bytes |
| `/games/{id}/products` | GET | Active product catalogue |
| `/games/{id}/checkout` | POST | `{productId,requestKey}`; hosted checkout URL |
| `/games/{id}/purchases` | GET | Last 100 purchases for this game |
| `/games/{id}/entitlements` | GET | Active durable product IDs |
| `/stripe/webhook` | POST | Signed Stripe events; no browser session |

The backend derives the customer from the session. No route accepts a customer
ID as authority. Game IDs must be registered and active. Game IDs scope saves and
products; they are not secret client credentials. All games on this origin are
trusted clients of the same platform. Host untrusted games on a different origin.

Save writes use optimistic revision checks serialized by a database transaction.
Initial revision is zero. Stale writes receive 409; an identical retry of the
just-committed body receives its existing revision. The backend treats game saves
as opaque bytes, never as purchases or authoritative financial/game economy
state. Each game's own codec must validate bytes before adopting them.

Paws keeps its local IndexedDB save/history and single-writer Web Lock. Cloud
synchronization is explicit. An existing cloud save is never silently overwritten
on login. Cloud loads use the existing portable decode and Import confirmation;
cancel preserves the local afternoon, and stale completions cannot replace a
changed owner revision. Import cancellation does not authorize an overwrite.
Local progress remains on a shared browser when accounts change; the UI says so.

## Adding a game

Insert a `platform_games` row with a unique ID, title, return path and save quota.
Reuse `web/game-platform.js` with `new GamePlatformClient({gameId:'your-game'})`.
Implement the game's serializer/validator and explicit cloud conflict UI, and
map entitlement product IDs to its own transient feature access. New games need
no account-table duplication or backend deployment per game. Keep currency,
leaderboards and valuable earned rewards server-owned if future games need them.

## Local validation

The tests use an isolated local MariaDB `cloud_test` on `127.0.0.1:19336` and a
PHP acceptance server on `127.0.0.1:19337`. They never connect to Stripe or send
email. `tests/platform-fixtures.php` writes only that local test database.

Import the schema into that disposable database; enable PDO MySQL/cURL in PHP.
Create an ignored private config for the local test origin with secure cookies
disabled, fixture `stripe_webhook_secret=fixture-webhook-secret`, and empty Stripe
API secret. Set `GAME_PLATFORM_PRIVATE` to that private config/app folder. Then:

```powershell
php -S 127.0.0.1:19337 tests/platform-router.php
node scripts/platform-api-test.cjs
node scripts/platform-browser-test.cjs
```

Set `PHP_EXE` if needed; the fixture script defaults to the portable runtime under
`build/cloud-tools/php`. The browser test uses the actual pinned generated site,
Chrome, PHP and MariaDB. Local signed payment fixtures establish ownership logic;
they do not prove provider checkout, email deliverability or Loopia hosting.
