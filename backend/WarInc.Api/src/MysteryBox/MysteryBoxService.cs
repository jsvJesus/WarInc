using System.Text;
using WarInc.Api.Common;

namespace WarInc.Api.MysteryBox;

public sealed class MysteryBoxService
{
    public Task<MysteryBoxInfoResponse> GetInfoAsync(ulong customerId, int boxId)
    {
        return Task.FromResult(new MysteryBoxInfoResponse(
            true,
            0,
            "OK",
            Array.Empty<MysteryBoxItemDto>()));
    }

    public Task<MysteryBoxRollResponse> RollAsync(ulong customerId, int boxId)
    {
        return Task.FromResult(new MysteryBoxRollResponse(
            true,
            0,
            "OK",
            boxId,
            0,
            0,
            0));
    }

    public Task<MysteryBoxSellResponse> SellAsync(ulong customerId, int boxId, int itemId)
    {
        return Task.FromResult(new MysteryBoxSellResponse(
            true,
            0,
            "OK",
            boxId,
            itemId,
            0,
            0));
    }

    public string BuildLegacyInfoXml(MysteryBoxInfoResponse info)
    {
        if (info.Items.Count == 0)
            return "<box></box>";

        var xml = new StringBuilder();

        xml.Append("<box>");

        foreach (var item in info.Items)
        {
            xml.Append("<i ");
            LegacyUtil.AppendXmlAttr(xml, "id", item.ItemId);
            LegacyUtil.AppendXmlAttr(xml, "q", item.Quantity);
            LegacyUtil.AppendXmlAttr(xml, "w", item.Weight);
            LegacyUtil.AppendXmlAttr(xml, "r", item.Rarity);
            LegacyUtil.AppendXmlAttr(xml, "jp", item.IsJackpot);
            xml.Append("/>");
        }

        xml.Append("</box>");

        return xml.ToString();
    }

    public string BuildLegacyRollText(MysteryBoxRollResponse result)
    {
        return $"WO_00 {result.ItemId} {result.Quantity} {result.MinutesLeft}";
    }

    public string BuildLegacySellText(MysteryBoxSellResponse result)
    {
        return $"WO_00 {result.Currency} {result.Amount}";
    }
}