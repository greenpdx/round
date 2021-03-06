/******************************************************************
 *
 * Round for C
 *
 * Copyright (C) Satoshi Konno 2015
 *
 * This is licensed under BSD-style license, see file COPYING.
 *
 ******************************************************************/

#include <boost/test/unit_test.hpp>

#include <round/util/base64.h>

ssize_t round_base64_encode(const byte *rawBytes, size_t rawByteLen, char **encodedStr);
ssize_t round_base64_decode(const char *encodedStr, byte **decordedBytes);

BOOST_AUTO_TEST_CASE(Base64Test)
{
  std::vector<std::string> testStrings;

  testStrings.push_back("");
  testStrings.push_back("a");
  testStrings.push_back("ab");
  testStrings.push_back("abc");
  testStrings.push_back("\b\t\r\n");
  testStrings.push_back("0123456789");
  testStrings.push_back("abcdefghijklmnopqrstuvwxyz");
  testStrings.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
  testStrings.push_back("+-=~`!@#$%^&*()_+{}[]|\\:;\"'<>,.?/");

  for (std::vector<std::string>::iterator testString = testStrings.begin(); testString != testStrings.end(); testString++) {
    byte *encordBytes = (byte *)(*testString).c_str();
    size_t ecordByteLen = (*testString).length();
    char *encodedStr = NULL;
    BOOST_CHECK(0 <= round_base64_encode(encordBytes, ecordByteLen, &encodedStr));

    byte *decordedBytes = NULL;
    size_t decordedByteLen = round_base64_decode(encodedStr, &decordedBytes);
    BOOST_CHECK(0 <= decordedByteLen);

    std::string decordedString((const char *)decordedBytes, decordedByteLen);
    BOOST_CHECK_EQUAL((*testString).compare(decordedString), 0);

    free(encodedStr);
    free(decordedBytes);
  }
}
