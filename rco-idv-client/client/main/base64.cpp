/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/
#include "base64.h"
#include "rc/rc_log.h"
#include "rc/rc_public.h"

namespace gloox
{

  namespace Base64
  {

    static const std::string alphabet64( "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" );
    static const char pad = '=';
    static const char np  = (char)std::string::npos;
    static char table64vals[] =
    {
      62, np, np, np, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, np, np, np, np, np,
      np, np,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17,
      18, 19, 20, 21, 22, 23, 24, 25, np, np, np, np, np, np, 26, 27, 28, 29, 30, 31,
      32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51
    };

    inline char table64( unsigned char c )
    {
      return ( c < 43 || c > 122 ) ? np : table64vals[c-43];
    }

    const std::string encode64( const std::string& input )
    {
      std::string encoded;
      char c;
      const std::string::size_type length = input.length();

      encoded.reserve( length * 2 );

      for( std::string::size_type i = 0; i < length; ++i )
      {
        c = static_cast<char>( ( input[i] >> 2 ) & 0x3f );
        encoded += alphabet64[c];

        c = static_cast<char>( ( input[i] << 4 ) & 0x3f );
        if( ++i < length )
          c = static_cast<char>( c | static_cast<char>( ( input[i] >> 4 ) & 0x0f ) );
        encoded += alphabet64[c];

        if( i < length )
        {
          c = static_cast<char>( ( input[i] << 2 ) & 0x3c );
          if( ++i < length )
            c = static_cast<char>( c | static_cast<char>( ( input[i] >> 6 ) & 0x03 ) );
          encoded += alphabet64[c];
        }
        else
        {
          ++i;
          encoded += pad;
        }

        if( i < length )
        {
          c = static_cast<char>( input[i] & 0x3f );
          encoded += alphabet64[c];
        }
        else
        {
          encoded += pad;
        }
      }

      return encoded;
    }

    const std::string decode64( const std::string& input )
    {
      char c, d;
      const std::string::size_type length = input.length();
      std::string decoded;

      decoded.reserve( length );

      for( std::string::size_type i = 0; i < length; ++i )
      {
        c = table64(input[i]);
        ++i;
        d = table64(input[i]);
        c = static_cast<char>( ( c << 2 ) | ( ( d >> 4 ) & 0x3 ) );
        decoded += c;
        if( ++i < length )
        {
          c = input[i];
          if( pad == c )
            break;

          c = table64(input[i]);
          d = static_cast<char>( ( ( d << 4 ) & 0xf0 ) | ( ( c >> 2 ) & 0xf ) );
          decoded += d;
        }

        if( ++i < length )
        {
          d = input[i];
          if( pad == d )
            break;

          d = table64(input[i]);
          c = static_cast<char>( ( ( c << 6 ) & 0xc0 ) | d );
          decoded += c;
        }
      }

      return decoded;
    }

  }

    static void add_linefeeds(string& text)
    {
        // add linefeeds every 64 characters
        int len = text.length();
        string temp_text;

        for (int i = 0; i < len; i += 64)
        {
            temp_text = temp_text + text.substr(i, 64) + '\n';
        }
        text = temp_text;
    }

    static void del_linefeeds(char* text, int len)
    {
        int i = 0, j = 0;
        for (i = 0; i < len; i++) {
            if (text[i] != '\n') {
                text[j++] = text[i];
            }
        }
        text[j] = '\0';
    }

    /**
     * Write password or ciphertext to KEY_FILE.
     */
    static void write_key_file(const string& input)
    {
        FILE *fp;
        fp = fopen(KEY_FILE.c_str(), "w");
        if (fp == NULL)
        {
            LOG_ERR("cannot open file %s", KEY_FILE.c_str());
            return;
        }
        fprintf(fp, "%s", input.c_str());
        fflush(fp);
        fsync(fileno(fp));
        fclose(fp);
    }

    string password_codec(const string& input, bool encode)
    {
        char command[512];
        char result[512];
        string temp_input = input;
        string output;
        int result_size = sizeof(result);
        int ret = 0;

        add_linefeeds(temp_input);
        write_key_file(temp_input);
        sprintf(command, "openssl aes-128-cbc %s -k %s -base64 -in %s" \
                , (encode ? "-e" : "-d") \
                , AES_KEY_STRING.c_str() \
                , KEY_FILE.c_str());
        if((ret = rc_system_rw(command, (unsigned char*)result, &result_size, "r")) != 0)
        {
            LOG_ERR("password_encode FAIL ret = %d", ret);
            output.clear();
        }
        else
        {
            del_linefeeds(result, result_size);
            output = result;
        }
        sprintf(command, "rm %s", KEY_FILE.c_str());
        rc_system(command);
        return output;
    }
    
    string password_codec_xor(const string& input, bool encode, string key)
    {
        int i = 0, key_len = 0, origin_len = 0;
        string output, out_xor;

        key_len = key.length();
        if (encode) {
            origin_len = input.length();
            for (i = 0; i < origin_len; i++) {
                out_xor += input[i] ^ key[i % key_len];
            }

            output = gloox::Base64::encode64(out_xor);
        } else {
            out_xor = gloox::Base64::decode64(input);
            origin_len = out_xor.length();
            for (i = 0; i < origin_len; i++) {
                output += out_xor[i] ^ key[i % key_len];
            }
        }

       // LOG_DEBUG("input %s out_xor %s out_xor_len %d origin_len %d output %s", input.c_str(), out_xor.c_str(), out_xor.length(), origin_len, output.c_str());
        return output;
    }
}
