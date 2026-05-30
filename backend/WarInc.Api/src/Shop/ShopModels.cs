namespace WarInc.Api.Shop;

public interface IHasPrices
{
    int Price1 { get; }
    int Price7 { get; }
    int Price30 { get; }
    int PricePerm { get; }

    int GPrice1 { get; }
    int GPrice7 { get; }
    int GPrice30 { get; }
    int GPricePerm { get; }
}

public interface ILegacyShopPrice : IHasPrices
{
    int ItemId { get; }
    int Category { get; }
}

public sealed class SkillPriceRow
{
    public int SkillId { get; set; }
    public int Lv1 { get; set; }
    public int Lv2 { get; set; }
    public int Lv3 { get; set; }
    public int Lv4 { get; set; }
    public int Lv5 { get; set; }
}

public sealed class LegacyShopPriceRow : ILegacyShopPrice
{
    public int ItemId { get; set; }
    public int Category { get; set; }

    public int Price1 { get; set; }
    public int Price7 { get; set; }
    public int Price30 { get; set; }
    public int PricePerm { get; set; }

    public int GPrice1 { get; set; }
    public int GPrice7 { get; set; }
    public int GPrice30 { get; set; }
    public int GPricePerm { get; set; }
}

public sealed class ShopItemRow : IHasPrices
{
    public int ItemId { get; set; }
    public int ItemType { get; set; }
    public int Category { get; set; }
    public int IsEnabled { get; set; }

    public string Name { get; set; } = "";
    public string FName { get; set; } = "";
    public string Description { get; set; } = "";

    public int LevelRequired { get; set; }

    public decimal Weight { get; set; }
    public decimal DamagePerc { get; set; }
    public decimal DamageMax { get; set; }

    public decimal Damage { get; set; }
    public decimal DamageDecay { get; set; }
    public int NumClips { get; set; }
    public int ClipSize { get; set; }
    public decimal RateOfFire { get; set; }
    public decimal Spread { get; set; }
    public decimal Recoil { get; set; }

    public int AddGP { get; set; }
    public int AddSP { get; set; }

    public int Item1Id { get; set; }
    public int Item1Exp { get; set; }
    public int Item2Id { get; set; }
    public int Item2Exp { get; set; }
    public int Item3Id { get; set; }
    public int Item3Exp { get; set; }
    public int Item4Id { get; set; }
    public int Item4Exp { get; set; }
    public int Item5Id { get; set; }
    public int Item5Exp { get; set; }
    public int Item6Id { get; set; }
    public int Item6Exp { get; set; }

    public int Price1 { get; set; }
    public int Price7 { get; set; }
    public int Price30 { get; set; }
    public int PricePerm { get; set; }

    public int GPrice1 { get; set; }
    public int GPrice7 { get; set; }
    public int GPrice30 { get; set; }
    public int GPricePerm { get; set; }
}

public sealed class GearItemRow : IHasPrices
{
    public int ItemId { get; set; }
    public int LevelRequired { get; set; }
    public decimal Weight { get; set; }
    public decimal DamagePerc { get; set; }
    public decimal DamageMax { get; set; }

    public int Price1 { get; set; }
    public int Price7 { get; set; }
    public int Price30 { get; set; }
    public int PricePerm { get; set; }

    public int GPrice1 { get; set; }
    public int GPrice7 { get; set; }
    public int GPrice30 { get; set; }
    public int GPricePerm { get; set; }
}

public sealed class WeaponItemRow : IHasPrices
{
    public int ItemId { get; set; }
    public int LevelRequired { get; set; }

    public decimal Damage { get; set; }
    public decimal DamageDecay { get; set; }
    public int NumClips { get; set; }
    public int ClipSize { get; set; }
    public decimal RateOfFire { get; set; }
    public decimal Spread { get; set; }
    public decimal Recoil { get; set; }

    public int Price1 { get; set; }
    public int Price7 { get; set; }
    public int Price30 { get; set; }
    public int PricePerm { get; set; }

    public int GPrice1 { get; set; }
    public int GPrice7 { get; set; }
    public int GPrice30 { get; set; }
    public int GPricePerm { get; set; }
}

public sealed class GenericItemRow : IHasPrices
{
    public int ItemId { get; set; }
    public int Category { get; set; }

    public string Name { get; set; } = "";
    public string FName { get; set; } = "";
    public string Description { get; set; } = "";

    public int Price1 { get; set; }
    public int Price7 { get; set; }
    public int Price30 { get; set; }
    public int PricePerm { get; set; }

    public int GPrice1 { get; set; }
    public int GPrice7 { get; set; }
    public int GPrice30 { get; set; }
    public int GPricePerm { get; set; }
}

public sealed class PackageItemRow : IHasPrices
{
    public int ItemId { get; set; }

    public string Name { get; set; } = "";
    public string FName { get; set; } = "";
    public string Description { get; set; } = "";

    public int AddGP { get; set; }
    public int AddSP { get; set; }

    public int Item1Id { get; set; }
    public int Item1Exp { get; set; }
    public int Item2Id { get; set; }
    public int Item2Exp { get; set; }
    public int Item3Id { get; set; }
    public int Item3Exp { get; set; }
    public int Item4Id { get; set; }
    public int Item4Exp { get; set; }
    public int Item5Id { get; set; }
    public int Item5Exp { get; set; }
    public int Item6Id { get; set; }
    public int Item6Exp { get; set; }

    public int Price1 { get; set; }
    public int Price7 { get; set; }
    public int Price30 { get; set; }
    public int PricePerm { get; set; }

    public int GPrice1 { get; set; }
    public int GPrice7 { get; set; }
    public int GPrice30 { get; set; }
    public int GPricePerm { get; set; }
}