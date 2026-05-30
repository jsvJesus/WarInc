using System.Text;
using WarInc.Api.Common;

namespace WarInc.Api.Retention;

public sealed class RetentionService
{
    public Task<RetentionInfoResponse> GetInfoAsync(ulong customerId)
    {
        return Task.FromResult(new RetentionInfoResponse(
            true,
            0,
            "OK",
            0,
            0,
            Array.Empty<RetentionDayDto>()));
    }

    public Task<RetentionClaimResponse> ClaimAsync(ulong customerId)
    {
        return Task.FromResult(new RetentionClaimResponse(
            true,
            0,
            "OK",
            0,
            Array.Empty<RetentionDayDto>()));
    }

    public string BuildLegacyInfoXml(RetentionInfoResponse info)
    {
        var xml = new StringBuilder();

        xml.Append("<retbonus ");
        LegacyUtil.AppendXmlAttr(xml, "d", info.CurrentDay);
        LegacyUtil.AppendXmlAttr(xml, "m", info.MaxDay);
        xml.Append(">");

        xml.Append("<days>");

        foreach (var day in info.Days)
        {
            xml.Append("<d ");
            LegacyUtil.AppendXmlAttr(xml, "i", day.Day);
            LegacyUtil.AppendXmlAttr(xml, "c", day.Claimed);
            LegacyUtil.AppendXmlAttr(xml, "rid", day.RewardId);
            LegacyUtil.AppendXmlAttr(xml, "amt", day.RewardAmount);
            LegacyUtil.AppendXmlAttr(xml, "type", day.RewardType);
            xml.Append("/>");
        }

        xml.Append("</days>");
        xml.Append("</retbonus>");

        return xml.ToString();
    }

    public string BuildLegacyEmptyInfoXml()
    {
        return "<retbonus d=\"0\" m=\"0\"><days></days></retbonus>";
    }
}