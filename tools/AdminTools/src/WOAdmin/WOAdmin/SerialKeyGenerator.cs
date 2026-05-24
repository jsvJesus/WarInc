using System;
using System.Collections.Generic;
using System.Text;
using System.Security.Cryptography;

namespace WOAdmin
{
    public enum SNKeyLength
    {
        SN16 = 16, SN20 = 20, SN24 = 24, SN28 = 28, SN32 = 32
    }

    public enum SNKeyNumLength
    {
        SN4 = 4, SN8 = 8, SN12 = 12
    }

    public static class RandomSNKGenerator
    {
        private static string AppendSpecifiedStr(int length, string str, char[] newKey)
        {
            string newKeyStr = "";
            int k = 0;
            for (int i = 0; i < length; i++)
            {
                for (k = i; k < 4 + i; k++)
                {
                    newKeyStr += newKey[k];
                }
                if (k == length)
                {
                    break;
                }
                else
                {
                    i = (k) - 1;
                    newKeyStr +=
                    str;
                }
            }
            return newKeyStr;
        }

        public static string GetSerialKeyAlphaNumaric(SNKeyLength keyLength)
        {
            Guid newguid = Guid.NewGuid();
            string randomStr = newguid.ToString("N");
            string tracStr = randomStr.Substring(0, (int)keyLength);
            tracStr = tracStr.ToUpper();
            char[] newKey = tracStr.ToCharArray();
            string newSerialNumber = "";
            switch (keyLength)
            {
                case SNKeyLength.SN16:
                    newSerialNumber = AppendSpecifiedStr(16, "-", newKey);
                    break;
                case SNKeyLength.SN20:
                    newSerialNumber = AppendSpecifiedStr(20, "-", newKey);
                    break;
                case SNKeyLength.SN24:
                    newSerialNumber = AppendSpecifiedStr(24, "-", newKey);
                    break;
                case SNKeyLength.SN28:
                    newSerialNumber = AppendSpecifiedStr(28, "-", newKey);
                    break;
                case SNKeyLength.SN32:
                    newSerialNumber = AppendSpecifiedStr(32, "-", newKey);
                    break;
            }
            return newSerialNumber;
        }

        public static string GetSerialKeyNumaric(SNKeyNumLength keyLength)
        {
            Random rn = new Random();
            double sd = Math.Round(rn.NextDouble() * Math.Pow(10, (int)keyLength) + 4);

            return sd.ToString().Substring(0, (int)keyLength);
        }

        public static string GetUniqueKey()
        {
            int maxSize = 16;
            char[] chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890".ToCharArray();
            byte[] data = new byte[maxSize];
            RNGCryptoServiceProvider crypto = new RNGCryptoServiceProvider();
            crypto.GetNonZeroBytes(data);
            StringBuilder result = new StringBuilder(maxSize);
            for(int i=0; i<maxSize; i++)
            {
                if (i > 0 && (i % 4) == 0)
                    result.Append("-");
                result.Append(chars[data[i] % (chars.Length - 1)]);
            }
            return result.ToString();
        }

        public static string GetUnique7SizeKey()
        {
            int maxSize = 7;
            char[] chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890".ToCharArray();
            byte[] data = new byte[maxSize];
            RNGCryptoServiceProvider crypto = new RNGCryptoServiceProvider();
            crypto.GetNonZeroBytes(data);
            StringBuilder result = new StringBuilder(maxSize);
            for (int i = 0; i < maxSize; i++)
            {
                result.Append(chars[data[i] % (chars.Length - 1)]);
            }
            return result.ToString();
        }

    }
}
