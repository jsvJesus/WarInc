using System.Globalization;
using System.Net;
using System.Text;

namespace WarInc.Api.Common;

public static class LegacyUtil
{
    public static ulong ParseULong(string value)
    {
        if (ulong.TryParse(value, NumberStyles.Integer, CultureInfo.InvariantCulture, out var result))
            return result;

        return 0;
    }

    public static int ParseInt(string value)
    {
        if (int.TryParse(value, NumberStyles.Integer, CultureInfo.InvariantCulture, out var result))
            return result;

        return 0;
    }

    public static int NormalizeLegacyErrorCode(int code)
    {
        if (code < 0)
            return 5;

        if (code > 9)
            return 9;

        return code;
    }

    public static void AppendXmlAttr(StringBuilder xml, string name, object? value)
    {
        var text = Convert.ToString(value, CultureInfo.InvariantCulture) ?? "";

        xml.Append(name);
        xml.Append("=\"");
        xml.Append(WebUtility.HtmlEncode(text));
        xml.Append("\" ");
    }

    public static void WriteInt32LE(MemoryStream stream, int value)
    {
        stream.WriteByte((byte)((value >> 0) & 0xFF));
        stream.WriteByte((byte)((value >> 8) & 0xFF));
        stream.WriteByte((byte)((value >> 16) & 0xFF));
        stream.WriteByte((byte)((value >> 24) & 0xFF));
    }

    public static void WriteUInt16LE(MemoryStream stream, int value)
    {
        stream.WriteByte((byte)((value >> 0) & 0xFF));
        stream.WriteByte((byte)((value >> 8) & 0xFF));
    }

    public static int ClampByte(int value)
    {
        if (value < 0)
            return 0;

        if (value > 255)
            return 255;

        return value;
    }
}