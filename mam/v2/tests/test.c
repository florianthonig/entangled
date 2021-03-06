/*
 * Copyright (c) 2018 IOTA Stiftung
 * https://github.com/iotaledger/entangled
 *
 * MAM is based on an original implementation & specification by apmi.bsu.by
 * [ITSec Lab]
 *
 * Refer to the LICENSE file for licensing information
 */

#include <time.h>

#include <unity/unity.h>

#include "mam/v2/tests/common.h"
#include "utils/macros.h"

void test_sponge_hash(size_t Xn, char *X, size_t Yn, char *Y) {
  test_sponge_t _s[1];
  isponge *s = test_sponge_init(_s);

  trits_t tX = trits_alloc(3 * Xn);
  trits_t tY = trits_alloc(3 * Yn);

  sponge_init(s);
  trytes_to_trits(X, tX.p, MIN(strlen(X), tX.n / RADIX));

  sponge_absorb(s, MAM2_SPONGE_CTL_DATA, tX);
  sponge_squeeze(s, MAM2_SPONGE_CTL_HASH, tY);
  trits_to_trytes(tY.p, Y, tY.n);

  trits_free(tX);
  trits_free(tY);
}

void test_sponge_encr(size_t Kn, char *K, size_t Xn, char *X, size_t Yn,
                      char *Y) {
  test_sponge_t _s[1];
  isponge *s = test_sponge_init(_s);

  trits_t tK = trits_alloc(3 * Kn);
  trits_t tX = trits_alloc(3 * Xn);
  trits_t tY = trits_alloc(3 * Yn);

  sponge_init(s);
  trytes_to_trits(K, tK.p, MIN(strlen(K), tK.n / RADIX));
  sponge_absorb(s, MAM2_SPONGE_CTL_KEY, tK);
  trytes_to_trits(X, tX.p, MIN(strlen(X), tX.n / RADIX));
  sponge_encr(s, tX, tY);
  trits_to_trytes(tY.p, Y, tY.n);

  trits_free(tK);
  trits_free(tX);
  trits_free(tY);
}

void test_sponge_decr(size_t Kn, char *K, size_t Yn, char *Y, size_t Xn,
                      char *X) {
  test_sponge_t _s[1];
  isponge *s = test_sponge_init(_s);

  trits_t tK = trits_alloc(3 * Kn);
  trits_t tY = trits_alloc(3 * Yn);
  trits_t tX = trits_alloc(3 * Xn);

  sponge_init(s);
  trytes_to_trits(K, tK.p, MIN(strlen(K), tK.n / RADIX));

  sponge_absorb(s, MAM2_SPONGE_CTL_KEY, tK);
  trytes_to_trits(Y, tY.p, MIN(strlen(Y), tY.n / RADIX));

  sponge_decr(s, tY, tX);
  trits_to_trytes(tX.p, X, tX.n);
  trits_free(tK);
  trits_free(tY);
  trits_free(tX);
}

void test_prng_gen(size_t Kn, char *K, trit_array_t const *const nonce,
                   size_t Yn, char *Y) {
  test_sponge_t _s[1];
  test_prng_t _p[1];
  isponge *s = test_sponge_init(_s);
  prng_t *p = test_prng_init(_p, s);

  trits_t tK = trits_alloc(3 * Kn);
  TRIT_ARRAY_DECLARE(tY, 3 * Yn);

  flex_trit_t key[FLEX_TRIT_SIZE_243];
  flex_trits_from_trytes(key, MAM2_PRNG_KEY_SIZE, K, HASH_LENGTH_TRYTE,
                         HASH_LENGTH_TRYTE);

  trytes_to_trits(K, tK.p, MIN(tK.n / RADIX, strlen(K)));
  prng_init(p, s, key);
  prng_gen(p, 0, nonce, &tY);
  flex_trits_to_trytes(Y, Yn, tY.trits, 3 * Yn, 3 * Yn);

  trits_free(tK);
}

void test_wots_gen_sign(size_t Kn, char *K, trit_array_t const *const nonce,
                        size_t pkn, char *pk, size_t Hn, char *H, size_t sign,
                        char *sig) {
  test_sponge_t _s[1];
  test_prng_t _p[1];
  test_wots_t _w[1];
  isponge *s = test_sponge_init(_s);
  prng_t *p = test_prng_init(_p, s);
  wots_t *w = test_wots_init(_w, s);

  trits_t tK = trits_alloc(3 * Kn);
  trits_t tpk = trits_alloc(3 * pkn);
  trits_t tH = trits_alloc(3 * Hn);
  trits_t tsig = trits_alloc(3 * sign);

  flex_trit_t key[MAM2_PRNG_KEY_SIZE];

  trytes_to_trits(K, tK.p, MIN(strlen(K), tK.n / RADIX));
  flex_trits_from_trytes(key, MAM2_PRNG_KEY_SIZE, K, HASH_LENGTH_TRYTE,
                         HASH_LENGTH_TRYTE);
  prng_init(p, s, key);

  wots_gen_sk(w, p, nonce);
  TRIT_ARRAY_MAKE_FROM_RAW(pk_trits_array, MAM2_WOTS_PK_SIZE, tpk.p + tpk.d);
  wots_calc_pk(w, &pk_trits_array);
  flex_trits_to_trits(tpk.p + tpk.d, MAM2_WOTS_PK_SIZE, pk_trits_array.trits,
                      MAM2_WOTS_PK_SIZE, MAM2_WOTS_PK_SIZE);

  trits_to_trytes(tpk.p, pk, MIN(strlen(pk), tpk.n / RADIX));

  trytes_to_trits(H, tH.p, MIN(strlen(H), tH.n));
  flex_trits_to_trits(tsig.p + tsig.d, MAM2_WOTS_SK_SIZE, w->sk,
                      MAM2_WOTS_SK_SIZE, MAM2_WOTS_SK_SIZE);
  TRIT_ARRAY_DECLARE(hash_array, MAM2_WOTS_HASH_SIZE);
  TRIT_ARRAY_DECLARE(sk_sig_array, MAM2_WOTS_SIG_SIZE);
  wots_sign(w, &hash_array, &sk_sig_array);
  flex_trits_to_trits(tsig.p + tsig.d, MAM2_WOTS_SIG_SIZE, sk_sig_array.trits,
                      MAM2_WOTS_SIG_SIZE, MAM2_WOTS_SIG_SIZE);

  trits_to_trytes(tsig.p, sig, MIN(tsig.n / RADIX, strlen(sig)));

  trits_free(tK);
  trits_free(tpk);
  trits_free(tH);
  trits_free(tsig);
}

void test() {
  tryte_t *nonce_trytes = (tryte_t *)"9ABCKNLMOXYZ";
  TRIT_ARRAY_DECLARE(nonce, 3 * 12);
  flex_trits_from_trytes(nonce.trits, 3 * 12, nonce_trytes, 12, 12);

  char K[81 + 1] =
      "ABCKNLMOXYZ99AZNODFGWERXCVH"
      "XCVHABCKNLMOXYZ99AZNODFGWER"
      "FGWERXCVHABCKNLMOXYZ99AZNOD";

  char H[78 + 1] =
      "BACKNLMOXYZ99AZNODFGWERXCVH"
      "XCVHABCKNLMOXYZ99AZNODFGWER"
      "FGWERXCVHABCKNLMOXYZ99AZ";

  char X[162 + 1] =
      "ABCKNLMOXYZ99AZNODFGWERXCVH"
      "XCVHABCKNLMOXYZ99AZNODFGWER"
      "FGWERXCVHABCKNLMOXYZ99AZNOD"
      "Z99AZNODFGABCKNLMOXYWERXCVH"
      "AZNODFGXCVKNLMOXHABCYZ99WER"
      "FGWERVHABCKXCNLM9AZNODOXYZ9";

  char *sponge_hash_expected =
      "Z9DDCLPZASLK9BCLPZASLKDVICXESYBIWBHJHQYOKIHNXHZDQVCFGDVIUTDVISKTMDG9EMON"
      "OYXPODPWU";
  char *sponge_encr_expected =
      "ABEZQN99JVWYZAZONRCHKUNKUSKSKSKTMDGQN99JVWYZAZONRCHKUNTYKUNKUSKTMDGQN99J"
      "VWYZAZONRNDAAZNODFGABCKNLMOXYWERXCVHAZNODFGXCVKNLMOXHABCYZ99WERFGWERVHAB"
      "CKXCNLM9AZNODOXYZ9";
  char *sponge_decr_expected =
      "ABSNTANLTJBBAAZPPERLQAVSLPEKVPEFUBITANLTJBBAAZPPERLQAVAPQAVSLPEFUBITANLT"
      "JBBAAZPPEKWZAZNODFGABCKNLMOXYWERXCVHAZNODFGXCVKNLMOXHABCYZ99WERFGWERVHAB"
      "CKXCNLM9AZNODOXYZ9";
  char *prng_gen_expected =
      "99MGDGQN99JVWYZZZNODFGWERXCVHXCVHABCKNLMOXYZ99AZNODFGWERFGWERXCVHABCKNLM"
      "OXYZ99AZNOD99ABCKNLMOXYZA99999999999999999999999999999999999999999999999"
      "999999999999999999";

  bool_t r = 1;
  size_t sponge_hash_Yn = 81;
  size_t sponge_encr_Yn = 162;
  size_t sponge_decr_Xn = 162;
  size_t prng_gen_Yn = 162;
  clock_t clk;

  char sponge_hash_Y[81];
  char sponge_encr_Y[162];
  char sponge_decr_X[162];
  char prng_gen_Y[162];
  char wots_gen_sign_pk[MAM2_WOTS_PK_SIZE / 3];
  char wots_gen_sign_sig[MAM2_WOTS_SIG_SIZE / 3];

  test_sponge_hash(162, X, sponge_hash_Yn, sponge_hash_Y);
  TEST_ASSERT_EQUAL_MEMORY(sponge_hash_Y, sponge_hash_expected, 81);

  test_sponge_encr(81, K, 162, X, sponge_encr_Yn, sponge_encr_Y);
  TEST_ASSERT_EQUAL_MEMORY(sponge_encr_Y, sponge_encr_expected, 162);

  test_sponge_decr(81, K, 162, X, sponge_decr_Xn, sponge_decr_X);
  TEST_ASSERT_EQUAL_MEMORY(sponge_decr_X, sponge_decr_expected, 162);

  test_prng_gen(81, K, &nonce, prng_gen_Yn, prng_gen_Y);
  TEST_ASSERT_EQUAL_MEMORY(prng_gen_Y, prng_gen_expected, 162);

  test_wots_gen_sign(81, K, &nonce, MAM2_WOTS_PK_SIZE / 3, wots_gen_sign_pk, 78,
                     H, MAM2_WOTS_SIG_SIZE / 3, wots_gen_sign_sig);

  // FIXME (@tsvisabo) test dec(enc(X)) = X
  // TODO - test/fix wots_gen_sign_sig
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test);

  return UNITY_END();
}
