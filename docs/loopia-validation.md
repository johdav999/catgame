# Loopia platform local validation — 18 October 2026

Implemented shared PHP/MariaDB accounts, password recovery, isolated game saves,
durable Stripe purchase tracking and Paws runtime/UI integration. No engine files
or gitlink were changed. Existing unrelated working-tree changes were preserved.

## Verified

- PHP 8.5.11 syntax checks pass for the application and public entry point.
- MariaDB 11.4.8 accepts the initial InnoDB schema in an isolated local database.
- `node scripts/platform-api-test.cjs`: **51 checks pass**, using real PHP/MariaDB,
  an injected checkout transport and locally signed Stripe event fixtures.
  Coverage includes accounts, CSRF/origin boundaries, customer/game isolation,
  optimistic save conflicts, identical retries, server-owned checkout prices and
  URLs, retry/order deduplication, cross-game request keys, paid/unpaid/duplicate
  events, forged/expired/wrong-mode signatures, amount mismatch, partial/full
  refunds, out-of-order refund/payment events, disputes, cross-game purchase
  history, single-use password reset and old-session revocation.
- `node scripts/platform-browser-test.cjs`: **17 checks pass** in Chrome
  155.0.8059.80 against the actual pinned generated site, PHP and MariaDB. Verified
  registration, save upload, existing-cloud replacement gate, cloud import cancel
  and confirm, IndexedDB reload, remembered account, verified purchase entitlement,
  the same account on another browser device, paid beach entry with development
  access disabled, sign-out while on the beach returning safely to the garden,
  retention of local progress, and a 390×844 account panel. No captured console or
  page errors. This viewport is emulation, not physical-device acceptance.
- Pinned Release browser build passes against WebEngine
  `3fe7d1dc1dc610c703944803cbb717cd65024ec9`.
  Build ID: `paws-0.1.0-30a755c647f0a408`.
- All **33 portable tests pass** across the main run and targeted rerun. Two
  historical save-test helpers incorrectly recognized only wrapper version 6;
  they now extract the legacy section from later wrappers as well. The large
  final-garden test passed in 154.61 seconds with a 240-second timeout after the
  initial 90-second timeout. The pinned portable scene test passed.
- `scripts/package-loopia.py` generated `build/loopia-platform-v1/` with the
  production runtime allowlist, public API/reset page, private application and
  config example, schema and deployment guide. Development preview targets and
  actual credentials are excluded. Packaged backend hash matches source.

Machine-readable API/browser evidence and a reviewed mobile-size screenshot are
under `build/evidence/platform/`. Browser evidence includes runtime payload hashes.

## Remaining service acceptance

No deployment to Loopia, real Stripe Checkout API call, Stripe-delivered webhook,
actual recovery email, live payment, physical mobile test or sale approval was
performed. Local checkout/event fixtures prove application behavior, not provider
configuration or hosted delivery. No product/price is enabled in the shipped
schema. The beach remains a development-content preview, separately from this
payment infrastructure. See [deployment and Stripe setup](loopia-platform.md).
