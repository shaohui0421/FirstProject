/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef BASE64_H__
#define BASE64_H__

#include <string>

namespace gloox
{

  /**
   * @brief An implementation of the Base64 data encoding (RFC 3548)
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.8
   */
  namespace Base64
  {

      /**
       * Base64-encodes the input according to RFC 3548.
       * @param input The data to encode.
       * @return The encoded string.
       */
      const std::string encode64( const std::string& input );

      /**
       * Base64-decodes the input according to RFC 3548.
       * @param input The encoded data.
       * @return The decoded data.
       */
      const std::string decode64( const std::string& input );

  }

    using std::string;

    const string AES_KEY_STRING = "FJRJroot_123";

    const string KEY_FILE = ".key";

    string password_codec(const string& input, bool encode);

    const string XOR_KEY_STRING = "YjU2ODJhNmZhNjQ";
    
    const string XOR_FTP_KEY_STRING = "ZGM1ODNhNTA0OGM";
    
    string password_codec_xor(const string& input, bool encode, string key = XOR_KEY_STRING);
}

#endif // BASE64_H__
