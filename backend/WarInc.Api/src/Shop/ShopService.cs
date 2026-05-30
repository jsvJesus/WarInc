using System.Text;
using Dapper;
using WarInc.Api.Common;
using WarInc.Api.Database;

namespace WarInc.Api.Shop;

public sealed class ShopService
{
    private readonly WarIncDb _db;

    public ShopService(WarIncDb db)
    {
        _db = db;
    }

    public async Task<object> GetShopItemsAsync()
    {
        await using var db = _db.CreateConnection();

        var gear = await db.QueryAsync<ShopItemRow>(
            """
            SELECT
                ItemID AS ItemId,
                1 AS ItemType,
                Category AS Category,
                1 AS IsEnabled,
                Name AS Name,
                FNAME AS FName,
                Description AS Description,
                LevelRequired AS LevelRequired,
                Weight AS Weight,
                DamagePerc AS DamagePerc,
                DamageMax AS DamageMax,
                0 AS Damage,
                0 AS DamageDecay,
                0 AS NumClips,
                0 AS ClipSize,
                60 AS RateOfFire,
                0 AS Spread,
                0 AS Recoil,
                0 AS AddGP,
                0 AS AddSP,
                0 AS Item1Id,
                0 AS Item1Exp,
                0 AS Item2Id,
                0 AS Item2Exp,
                0 AS Item3Id,
                0 AS Item3Exp,
                0 AS Item4Id,
                0 AS Item4Exp,
                0 AS Item5Id,
                0 AS Item5Exp,
                0 AS Item6Id,
                0 AS Item6Exp,
                Price1 AS Price1,
                Price7 AS Price7,
                Price30 AS Price30,
                PriceP AS PricePerm,
                GPrice1 AS GPrice1,
                GPrice7 AS GPrice7,
                GPrice30 AS GPrice30,
                GPriceP AS GPricePerm
            FROM items_gear
            ORDER BY ItemID;
            """);

        var weapons = await db.QueryAsync<ShopItemRow>(
            """
            SELECT
                ItemID AS ItemId,
                2 AS ItemType,
                Category AS Category,
                1 AS IsEnabled,
                Name AS Name,
                FNAME AS FName,
                Description AS Description,
                LevelRequired AS LevelRequired,
                0 AS Weight,
                0 AS DamagePerc,
                0 AS DamageMax,
                Damage AS Damage,
                DamageDecay AS DamageDecay,
                NumClips AS NumClips,
                Clipsize AS ClipSize,
                RateOfFire AS RateOfFire,
                Spread AS Spread,
                Recoil AS Recoil,
                0 AS AddGP,
                0 AS AddSP,
                0 AS Item1Id,
                0 AS Item1Exp,
                0 AS Item2Id,
                0 AS Item2Exp,
                0 AS Item3Id,
                0 AS Item3Exp,
                0 AS Item4Id,
                0 AS Item4Exp,
                0 AS Item5Id,
                0 AS Item5Exp,
                0 AS Item6Id,
                0 AS Item6Exp,
                Price1 AS Price1,
                Price7 AS Price7,
                Price30 AS Price30,
                PriceP AS PricePerm,
                GPrice1 AS GPrice1,
                GPrice7 AS GPrice7,
                GPrice30 AS GPrice30,
                GPriceP AS GPricePerm
            FROM items_weapons
            ORDER BY ItemID;
            """);

        var generics = await db.QueryAsync<ShopItemRow>(
            """
            SELECT
                ItemID AS ItemId,
                3 AS ItemType,
                Category AS Category,
                1 AS IsEnabled,
                Name AS Name,
                FNAME AS FName,
                Description AS Description,
                LevelRequired AS LevelRequired,
                0 AS Weight,
                0 AS DamagePerc,
                0 AS DamageMax,
                0 AS Damage,
                0 AS DamageDecay,
                0 AS NumClips,
                0 AS ClipSize,
                60 AS RateOfFire,
                0 AS Spread,
                0 AS Recoil,
                0 AS AddGP,
                0 AS AddSP,
                0 AS Item1Id,
                0 AS Item1Exp,
                0 AS Item2Id,
                0 AS Item2Exp,
                0 AS Item3Id,
                0 AS Item3Exp,
                0 AS Item4Id,
                0 AS Item4Exp,
                0 AS Item5Id,
                0 AS Item5Exp,
                0 AS Item6Id,
                0 AS Item6Exp,
                Price1 AS Price1,
                Price7 AS Price7,
                Price30 AS Price30,
                PriceP AS PricePerm,
                GPrice1 AS GPrice1,
                GPrice7 AS GPrice7,
                GPrice30 AS GPrice30,
                GPriceP AS GPricePerm
            FROM items_generic
            ORDER BY ItemID;
            """);

        var attachments = await db.QueryAsync<ShopItemRow>(
            """
            SELECT
                ItemID AS ItemId,
                5 AS ItemType,
                Category AS Category,
                1 AS IsEnabled,
                Name AS Name,
                FNAME AS FName,
                Description AS Description,
                LevelRequired AS LevelRequired,
                0 AS Weight,
                0 AS DamagePerc,
                0 AS DamageMax,
                Damage AS Damage,
                0 AS DamageDecay,
                0 AS NumClips,
                Clipsize AS ClipSize,
                Firerate AS RateOfFire,
                Spread AS Spread,
                Recoil AS Recoil,
                0 AS AddGP,
                0 AS AddSP,
                0 AS Item1Id,
                0 AS Item1Exp,
                0 AS Item2Id,
                0 AS Item2Exp,
                0 AS Item3Id,
                0 AS Item3Exp,
                0 AS Item4Id,
                0 AS Item4Exp,
                0 AS Item5Id,
                0 AS Item5Exp,
                0 AS Item6Id,
                0 AS Item6Exp,
                Price1 AS Price1,
                Price7 AS Price7,
                Price30 AS Price30,
                PriceP AS PricePerm,
                GPrice1 AS GPrice1,
                GPrice7 AS GPrice7,
                GPrice30 AS GPrice30,
                GPriceP AS GPricePerm
            FROM items_attachments
            ORDER BY ItemID;
            """);

        var packages = await db.QueryAsync<ShopItemRow>(
            """
            SELECT
                ItemID AS ItemId,
                4 AS ItemType,
                Category AS Category,
                IsEnabled AS IsEnabled,
                Name AS Name,
                FNAME AS FName,
                Description AS Description,
                LevelRequired AS LevelRequired,
                0 AS Weight,
                0 AS DamagePerc,
                0 AS DamageMax,
                0 AS Damage,
                0 AS DamageDecay,
                0 AS NumClips,
                0 AS ClipSize,
                60 AS RateOfFire,
                0 AS Spread,
                0 AS Recoil,
                AddGP AS AddGP,
                AddSP AS AddSP,
                Item1_ID AS Item1Id,
                Item1_Exp AS Item1Exp,
                Item2_ID AS Item2Id,
                Item2_Exp AS Item2Exp,
                Item3_ID AS Item3Id,
                Item3_Exp AS Item3Exp,
                Item4_ID AS Item4Id,
                Item4_Exp AS Item4Exp,
                Item5_ID AS Item5Id,
                Item5_Exp AS Item5Exp,
                Item6_ID AS Item6Id,
                Item6_Exp AS Item6Exp,
                Price1 AS Price1,
                Price7 AS Price7,
                Price30 AS Price30,
                PriceP AS PricePerm,
                GPrice1 AS GPrice1,
                GPrice7 AS GPrice7,
                GPrice30 AS GPrice30,
                GPriceP AS GPricePerm
            FROM items_packages
            ORDER BY ItemID;
            """);

        return new
        {
            ok = true,
            gear,
            weapons,
            generics,
            attachments,
            packages
        };
    }

    public async Task<byte[]> BuildLegacyShop5Async()
    {
        await using var db = _db.CreateConnection();

        var skills = (await db.QueryAsync<SkillPriceRow>(
            """
            SELECT
                SkillID AS SkillId,
                IFNULL(Lv1, 0) AS Lv1,
                IFNULL(Lv2, 0) AS Lv2,
                IFNULL(Lv3, 0) AS Lv3,
                IFNULL(Lv4, 0) AS Lv4,
                IFNULL(Lv5, 0) AS Lv5
            FROM dataskill2price
            ORDER BY SkillID;
            """)).ToList();

        var commonItems = (await db.QueryAsync<LegacyShopPriceRow>(
            """
            SELECT ItemID AS ItemId, Category AS Category, Price1 AS Price1, Price7 AS Price7, Price30 AS Price30, PriceP AS PricePerm, GPrice1 AS GPrice1, GPrice7 AS GPrice7, GPrice30 AS GPrice30, GPriceP AS GPricePerm FROM items_gear
            UNION ALL
            SELECT ItemID AS ItemId, Category AS Category, Price1 AS Price1, Price7 AS Price7, Price30 AS Price30, PriceP AS PricePerm, GPrice1 AS GPrice1, GPrice7 AS GPrice7, GPrice30 AS GPrice30, GPriceP AS GPricePerm FROM items_weapons
            UNION ALL
            SELECT ItemID AS ItemId, Category AS Category, Price1 AS Price1, Price7 AS Price7, Price30 AS Price30, PriceP AS PricePerm, GPrice1 AS GPrice1, GPrice7 AS GPrice7, GPrice30 AS GPrice30, GPriceP AS GPricePerm FROM items_generic
            UNION ALL
            SELECT ItemID AS ItemId, Category AS Category, Price1 AS Price1, Price7 AS Price7, Price30 AS Price30, PriceP AS PricePerm, GPrice1 AS GPrice1, GPrice7 AS GPrice7, GPrice30 AS GPrice30, GPriceP AS GPricePerm FROM items_attachments
            ORDER BY ItemId;
            """)).ToList();

        var packages = (await db.QueryAsync<LegacyShopPriceRow>(
            """
            SELECT
                ItemID AS ItemId,
                Category AS Category,
                Price1 AS Price1,
                Price7 AS Price7,
                Price30 AS Price30,
                PriceP AS PricePerm,
                GPrice1 AS GPrice1,
                GPrice7 AS GPrice7,
                GPrice30 AS GPrice30,
                GPriceP AS GPricePerm
            FROM items_packages
            WHERE IsEnabled <> 0
            ORDER BY ItemID;
            """)).ToList();

        var shopData = new MemoryStream();
        var header = Encoding.ASCII.GetBytes("SHO3");

        shopData.Write(header, 0, header.Length);

        foreach (var skill in skills)
        {
            LegacyUtil.WriteUInt16LE(shopData, skill.SkillId);
            shopData.WriteByte((byte)LegacyUtil.ClampByte(skill.Lv1));
            shopData.WriteByte((byte)LegacyUtil.ClampByte(skill.Lv2));
            shopData.WriteByte((byte)LegacyUtil.ClampByte(skill.Lv3));
            shopData.WriteByte((byte)LegacyUtil.ClampByte(skill.Lv4));
            shopData.WriteByte((byte)LegacyUtil.ClampByte(skill.Lv5));
        }

        LegacyUtil.WriteUInt16LE(shopData, 0xFFFF);

        shopData.Write(header, 0, header.Length);

        foreach (var item in commonItems)
            WriteLegacyShopItem(shopData, item);

        foreach (var item in packages)
            WriteLegacyShopItem(shopData, item);

        shopData.Write(header, 0, header.Length);

        var prefix = Encoding.ASCII.GetBytes("WO_0");
        var payload = shopData.ToArray();

        var result = new byte[prefix.Length + payload.Length];

        Buffer.BlockCopy(prefix, 0, result, 0, prefix.Length);
        Buffer.BlockCopy(payload, 0, result, prefix.Length, payload.Length);

        return result;
    }

    public async Task<string> BuildLegacyItemsInfoXmlAsync()
    {
        await using var db = _db.CreateConnection();

        var gears = (await db.QueryAsync<GearItemRow>(
            """
            SELECT
                ItemID AS ItemId,
                LevelRequired AS LevelRequired,
                Weight AS Weight,
                DamagePerc AS DamagePerc,
                DamageMax AS DamageMax,
                Price1 AS Price1,
                Price7 AS Price7,
                Price30 AS Price30,
                PriceP AS PricePerm,
                GPrice1 AS GPrice1,
                GPrice7 AS GPrice7,
                GPrice30 AS GPrice30,
                GPriceP AS GPricePerm
            FROM items_gear
            ORDER BY ItemID;
            """)).ToList();

        var weapons = (await db.QueryAsync<WeaponItemRow>(
            """
            SELECT
                ItemID AS ItemId,
                LevelRequired AS LevelRequired,
                Damage AS Damage,
                DamageDecay AS DamageDecay,
                NumClips AS NumClips,
                Clipsize AS ClipSize,
                RateOfFire AS RateOfFire,
                Spread AS Spread,
                Recoil AS Recoil,
                Price1 AS Price1,
                Price7 AS Price7,
                Price30 AS Price30,
                PriceP AS PricePerm,
                GPrice1 AS GPrice1,
                GPrice7 AS GPrice7,
                GPrice30 AS GPrice30,
                GPriceP AS GPricePerm
            FROM items_weapons
            ORDER BY ItemID;
            """)).ToList();

        var generics = (await db.QueryAsync<GenericItemRow>(
            """
            SELECT
                ItemID AS ItemId,
                Category AS Category,
                Name AS Name,
                FNAME AS FName,
                Description AS Description,
                Price1 AS Price1,
                Price7 AS Price7,
                Price30 AS Price30,
                PriceP AS PricePerm,
                GPrice1 AS GPrice1,
                GPrice7 AS GPrice7,
                GPrice30 AS GPrice30,
                GPriceP AS GPricePerm
            FROM items_generic
            ORDER BY ItemID;
            """)).ToList();

        var packages = (await db.QueryAsync<PackageItemRow>(
            """
            SELECT
                ItemID AS ItemId,
                Name AS Name,
                FNAME AS FName,
                Description AS Description,
                AddGP AS AddGP,
                AddSP AS AddSP,
                Item1_ID AS Item1Id,
                Item1_Exp AS Item1Exp,
                Item2_ID AS Item2Id,
                Item2_Exp AS Item2Exp,
                Item3_ID AS Item3Id,
                Item3_Exp AS Item3Exp,
                Item4_ID AS Item4Id,
                Item4_Exp AS Item4Exp,
                Item5_ID AS Item5Id,
                Item5_Exp AS Item5Exp,
                Item6_ID AS Item6Id,
                Item6_Exp AS Item6Exp,
                Price1 AS Price1,
                Price7 AS Price7,
                Price30 AS Price30,
                PriceP AS PricePerm,
                GPrice1 AS GPrice1,
                GPrice7 AS GPrice7,
                GPrice30 AS GPrice30,
                GPriceP AS GPricePerm
            FROM items_packages
            WHERE IsEnabled <> 0
            ORDER BY ItemID;
            """)).ToList();

        var xml = new StringBuilder();

        xml.Append("<?xml version=\"1.0\"?>\n");
        xml.Append("<items>");

        xml.Append("<gears>");
        foreach (var item in gears)
        {
            if (!HasAnyPrice(item))
                continue;

            xml.Append("<g ");
            LegacyUtil.AppendXmlAttr(xml, "ID", item.ItemId);
            LegacyUtil.AppendXmlAttr(xml, "lv", item.LevelRequired);
            LegacyUtil.AppendXmlAttr(xml, "wg", item.Weight);
            LegacyUtil.AppendXmlAttr(xml, "dp", item.DamagePerc);
            LegacyUtil.AppendXmlAttr(xml, "dm", item.DamageMax);
            xml.Append("/>\n");
        }
        xml.Append("</gears>");

        xml.Append("<weapons>");
        foreach (var item in weapons)
        {
            if (!HasAnyPrice(item))
                continue;

            xml.Append("<w ");
            LegacyUtil.AppendXmlAttr(xml, "ID", item.ItemId);
            LegacyUtil.AppendXmlAttr(xml, "lv", item.LevelRequired);
            LegacyUtil.AppendXmlAttr(xml, "d1", item.Damage);
            LegacyUtil.AppendXmlAttr(xml, "d2", item.DamageDecay);
            LegacyUtil.AppendXmlAttr(xml, "c1", item.NumClips);
            LegacyUtil.AppendXmlAttr(xml, "c2", item.ClipSize);
            LegacyUtil.AppendXmlAttr(xml, "rf", item.RateOfFire);
            LegacyUtil.AppendXmlAttr(xml, "sp", item.Spread);
            LegacyUtil.AppendXmlAttr(xml, "rc", item.Recoil);
            xml.Append("/>\n");
        }
        xml.Append("</weapons>");

        xml.Append("<generics>");
        foreach (var item in generics)
        {
            if (!HasAnyPrice(item))
                continue;

            if (item.Category != 7 && item.Category != 3)
                continue;

            xml.Append("<b ");
            LegacyUtil.AppendXmlAttr(xml, "ID", item.ItemId);
            LegacyUtil.AppendXmlAttr(xml, "name", item.Name);
            LegacyUtil.AppendXmlAttr(xml, "fname", item.FName);
            LegacyUtil.AppendXmlAttr(xml, "desc", item.Description);
            xml.Append("/>\n");
        }
        xml.Append("</generics>");

        xml.Append("<packages>");
        foreach (var item in packages)
        {
            if (!HasAnyPrice(item))
                continue;

            xml.Append("<p ");
            LegacyUtil.AppendXmlAttr(xml, "ID", item.ItemId);
            LegacyUtil.AppendXmlAttr(xml, "name", item.Name);
            LegacyUtil.AppendXmlAttr(xml, "fname", item.FName);
            LegacyUtil.AppendXmlAttr(xml, "desc", item.Description);
            LegacyUtil.AppendXmlAttr(xml, "gd", item.AddGP);
            LegacyUtil.AppendXmlAttr(xml, "sp", item.AddSP);
            LegacyUtil.AppendXmlAttr(xml, "i1i", item.Item1Id);
            LegacyUtil.AppendXmlAttr(xml, "i1e", item.Item1Exp);
            LegacyUtil.AppendXmlAttr(xml, "i2i", item.Item2Id);
            LegacyUtil.AppendXmlAttr(xml, "i2e", item.Item2Exp);
            LegacyUtil.AppendXmlAttr(xml, "i3i", item.Item3Id);
            LegacyUtil.AppendXmlAttr(xml, "i3e", item.Item3Exp);
            LegacyUtil.AppendXmlAttr(xml, "i4i", item.Item4Id);
            LegacyUtil.AppendXmlAttr(xml, "i4e", item.Item4Exp);
            LegacyUtil.AppendXmlAttr(xml, "i5i", item.Item5Id);
            LegacyUtil.AppendXmlAttr(xml, "i5e", item.Item5Exp);
            LegacyUtil.AppendXmlAttr(xml, "i6i", item.Item6Id);
            LegacyUtil.AppendXmlAttr(xml, "i6e", item.Item6Exp);
            xml.Append("/>\n");
        }
        xml.Append("</packages>");

        xml.Append("</items>");

        return xml.ToString();
    }

    private static void WriteLegacyShopItem(MemoryStream stream, ILegacyShopPrice item)
    {
        var priceBits = GetPriceBits(item);

        if (priceBits == 0)
            return;

        LegacyUtil.WriteInt32LE(stream, item.ItemId);
        stream.WriteByte((byte)priceBits);
        stream.WriteByte((byte)item.Category);

        if ((priceBits & 1) != 0)
            LegacyUtil.WriteInt32LE(stream, item.Price1);

        if ((priceBits & 2) != 0)
            LegacyUtil.WriteInt32LE(stream, item.Price7);

        if ((priceBits & 4) != 0)
            LegacyUtil.WriteInt32LE(stream, item.Price30);

        if ((priceBits & 8) != 0)
            LegacyUtil.WriteInt32LE(stream, item.PricePerm);

        if ((priceBits & 16) != 0)
            LegacyUtil.WriteInt32LE(stream, item.GPrice1);

        if ((priceBits & 32) != 0)
            LegacyUtil.WriteInt32LE(stream, item.GPrice7);

        if ((priceBits & 64) != 0)
            LegacyUtil.WriteInt32LE(stream, item.GPrice30);

        if ((priceBits & 128) != 0)
            LegacyUtil.WriteInt32LE(stream, item.GPricePerm);
    }

    private static int GetPriceBits(IHasPrices item)
    {
        var bits = 0;

        if (item.Price1 > 0)
            bits |= 1;

        if (item.Price7 > 0)
            bits |= 2;

        if (item.Price30 > 0)
            bits |= 4;

        if (item.PricePerm > 0)
            bits |= 8;

        if (item.GPrice1 > 0)
            bits |= 16;

        if (item.GPrice7 > 0)
            bits |= 32;

        if (item.GPrice30 > 0)
            bits |= 64;

        if (item.GPricePerm > 0)
            bits |= 128;

        return bits;
    }

    private static bool HasAnyPrice(IHasPrices item)
    {
        return GetPriceBits(item) != 0;
    }
}