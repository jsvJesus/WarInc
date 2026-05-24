using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Data.SqlClient;

namespace WOAdmin
{
    /*
    FNAME	varchar(32)	
    Category	int	
    Name	varchar(32)	
    Description	varchar(256)	
    MuzzleOffset	varchar(32)	
    MuzzleParticle	varchar(32)	
    Animation	varchar(32)	
    BulletID	varchar(32)	
    Sound_Shot	varchar(255)
    Sound_Reload	varchar(255)	
    Damage	float	
    isImmediate	int	
    Mass	float	
    Speed	float	
    DamageDecay	float	
    Area	float	
    Delay	float	
    Timeout	float	
    NumClips	int	
    Clipsize	int	
    ReloadTime	float	
    ActiveReloadTick	float	
    RateOfFire	int	
    Spread	float	
    Recoil	float	
    ScopeType varchar(32)
    ScopeZoom int
         */

    public class DBWeapon : DBCommonItem
    {
        public static bool IS_THIS_CATEGORY(int category)
        {
            return category >= 20 && category <= 29;
        }

        public static EditField_Combo.SPairs CategoryValues = new EditField_Combo.SPairs
        {
            new EditField_Combo.SPair("20", "Assault Rifle"), 
            new EditField_Combo.SPair("21", "Sniper Rifle"), 
            new EditField_Combo.SPair("22", "Shotgun"), 
            new EditField_Combo.SPair("23", "Machine gun"), 
            new EditField_Combo.SPair("24", "RPG"), 
            new EditField_Combo.SPair("25", "Handguns"), 
            new EditField_Combo.SPair("26", "Submachinegun"), 
            new EditField_Combo.SPair("27", "Grenade"), 
            new EditField_Combo.SPair("28", "Usable Item"),
            new EditField_Combo.SPair("29", "Melee"), 
        };

        public static EditField_Combo.SPairs AnimationValues = new EditField_Combo.SPairs
        {
            new EditField_Combo.SPair("pistol", "Pistol"), 
            new EditField_Combo.SPair("assault", "Assault"), 
            new EditField_Combo.SPair("smg", "SMG"), 
            new EditField_Combo.SPair("grenade", "Grenade"), 
            new EditField_Combo.SPair("rpg", "RPG"), 
            new EditField_Combo.SPair("mine", "Mine"), 
            new EditField_Combo.SPair("melee", "Melee"), 
        };

        public static EditField_Combo.SPairs BulletIDValues = new EditField_Combo.SPairs
        {
            new EditField_Combo.SPair("5.45", "5.45"), 
            new EditField_Combo.SPair("Sniper5.45", "RealBullet"), 
            new EditField_Combo.SPair("5.56", "5.56"), 
            new EditField_Combo.SPair("7.62", "7.62"), 
            new EditField_Combo.SPair("9mm", "9mm"), 
            new EditField_Combo.SPair(".40", ".40"), 
            new EditField_Combo.SPair(".45", ".45"), 
            new EditField_Combo.SPair(".5", ".5"), 
            new EditField_Combo.SPair(".38", ".38"), 
            new EditField_Combo.SPair("buckshot", "buckshot"), 
            new EditField_Combo.SPair("slug", "slug"), 
            new EditField_Combo.SPair("melee", "melee"), 
            
            new EditField_Combo.SPair("rocket", "rocket"), 
            new EditField_Combo.SPair("Rocket_AT4", "Rocket_AT4"), 
            new EditField_Combo.SPair("Rocket_M202", "Rocket_M202"), 
            new EditField_Combo.SPair("Rocket_MPISRAW", "Rocket_MPISRAW"), 
            new EditField_Combo.SPair("Rocket_RG6", "Rocket_RG6"), 
            new EditField_Combo.SPair("Rocket_RPG26", "Rocket_RPG26"), 
            new EditField_Combo.SPair("Rocket_RPO", "Rocket_RPO"), 

            new EditField_Combo.SPair("grenade", "grenade"), 
            new EditField_Combo.SPair("WhitePhosphorus", "WhitePhosphorus"), 
            new EditField_Combo.SPair("FlashBang", "FlashBang"), 
            new EditField_Combo.SPair("ClaymoreMine", "ClaymoreMine"), 
            new EditField_Combo.SPair("RegularMine", "RegularMine"), 
            new EditField_Combo.SPair("SatchelCharge", "SatchelCharge"), 
            new EditField_Combo.SPair("smoke_grenade_blue", "smoke_grenade_blue"),
            new EditField_Combo.SPair("smoke_grenade_green", "smoke_grenade_green"),
            new EditField_Combo.SPair("smoke_grenade_orange", "smoke_grenade_orange"),
            new EditField_Combo.SPair("smoke_grenade_red", "smoke_grenade_red"),
            new EditField_Combo.SPair("smoke_grenade_white", "smoke_grenade_white"),
        };
        
        public static EditField_Combo.SPairs FiremodeValues = new EditField_Combo.SPairs
        {
            new EditField_Combo.SPair("100", "Single"), 
            new EditField_Combo.SPair("110", "Single+Triple"), 
            new EditField_Combo.SPair("111", "Single+Triple+Auto"), 
            new EditField_Combo.SPair("101", "Single+Auto"), 
            new EditField_Combo.SPair("010", "Triple"), 
            new EditField_Combo.SPair("011", "Triple+Auto"), 
            new EditField_Combo.SPair("001", "Auto"), 
        };

        public static EditField_Combo.SPairs GrenadeNameValues = new EditField_Combo.SPairs
        {
            new EditField_Combo.SPair("asr_grenade", "asr_grenade"), 
        };

        public static EditField_Combo.SPairs MuzzleParticleValues = new EditField_Combo.SPairs
        {
            new EditField_Combo.SPair("", "none"), 
            new EditField_Combo.SPair("muzzle_asr", "muzzle_asr"), 
            new EditField_Combo.SPair("muzzle_hg", "muzzle_hg"), 
            new EditField_Combo.SPair("muzzle_rpg", "muzzle_rpg"), 
            new EditField_Combo.SPair("muzzle_sh1", "muzzle_sh1"), 
        };

        public static EditField_Combo.SPairs SoundShotValues = new EditField_Combo.SPairs
        {
            new EditField_Combo.SPair("", "none"),

            // OLD SOUNDS
            new EditField_Combo.SPair("Sounds/Weapons/ASR/ASR_Generic_Shot", "ASR Generic"),
            new EditField_Combo.SPair("Sounds/Weapons/SUPPORT/SUP_Generic_Shot", "SUP Generic "),
            new EditField_Combo.SPair("Sounds/Weapons/HG/HG_Generic_Shot", "HG Generic "),
            new EditField_Combo.SPair("Sounds/Weapons/SNP/SNP_Generic_Shot", "SNP Generic "),
            new EditField_Combo.SPair("Sounds/Weapons/MG/MG_Generic_Shot", "MG Generic "),
            new EditField_Combo.SPair("Sounds/Weapons/SMG/SMG_Generic_Shot", "SMG Generic "),

            new EditField_Combo.SPair("Sounds/Weapons/SNP/SNP_SVD_SHOT", "OLD Sniper SVD"), // sergey really likes it, so do not delete

            // NEW SOUNDS
            new EditField_Combo.SPair("Sounds/NewWeapons/Assault/AEK971", "ASR AEK971"),
            new EditField_Combo.SPair("Sounds/NewWeapons/Assault/AKS-74U", "ASR AKS-74U"),
            new EditField_Combo.SPair("Sounds/NewWeapons/Assault/ColtM16", "ASR Colt M16"),
            new EditField_Combo.SPair("Sounds/NewWeapons/Assault/ColtM4", "ASR Colt M4"),
            new EditField_Combo.SPair("Sounds/NewWeapons/Assault/FN_FAL", "ASR FN FAL"),
            new EditField_Combo.SPair("Sounds/NewWeapons/Assault/FN_SCAR", "ASR FN SCAR"),
            new EditField_Combo.SPair("Sounds/NewWeapons/Assault/Famas_F1", "ASR Famas F1"),
            new EditField_Combo.SPair("Sounds/NewWeapons/Assault/HK_416", "ASR HK 416"),
            new EditField_Combo.SPair("Sounds/NewWeapons/Assault/HK_G11", "ASR HK G11"),
            new EditField_Combo.SPair("Sounds/NewWeapons/Assault/HK_G36_CMag", "ASR HK G36 CMag"),
            new EditField_Combo.SPair("Sounds/NewWeapons/Assault/HK_G36", "ASR HK G36"),
            new EditField_Combo.SPair("Sounds/NewWeapons/Assault/Kalashnikov_AK108", "ASR AK108"),
            new EditField_Combo.SPair("Sounds/NewWeapons/Assault/Kalashnikov_AK74", "ASR AK74"),
            new EditField_Combo.SPair("Sounds/NewWeapons/Assault/Kalashnikov_AKM", "ASR AKM"),
            new EditField_Combo.SPair("Sounds/NewWeapons/Assault/SIG_551", "ASR SIG 551"),
            new EditField_Combo.SPair("Sounds/NewWeapons/Assault/XM8", "ASR XM8"),

            new EditField_Combo.SPair("Sounds/NewWeapons/Handgun/0.45Generic", "HG .45 Generic"),
            new EditField_Combo.SPair("Sounds/NewWeapons/Handgun/9mmGeneric", "HG 9mm Generic"),
            new EditField_Combo.SPair("Sounds/NewWeapons/Handgun/Beretta92", "HG Beretta92"),
            new EditField_Combo.SPair("Sounds/NewWeapons/Handgun/Beretta93R", "HG Beretta93R"),
            new EditField_Combo.SPair("Sounds/NewWeapons/Handgun/DesertEagle", "HG Desert Eagle"),
            new EditField_Combo.SPair("Sounds/NewWeapons/Handgun/H&K_USP", "HG H&K USP"),
            new EditField_Combo.SPair("Sounds/NewWeapons/Handgun/Glock9mm", "HG Glock9mm"),

            new EditField_Combo.SPair("Sounds/NewWeapons/LightMachineGun/FN_M249", "LMG FN M249"),
            new EditField_Combo.SPair("Sounds/NewWeapons/LightMachineGun/Kalashnikov_PKM", "LMG PKM"),
            new EditField_Combo.SPair("Sounds/NewWeapons/LightMachineGun/Kalashnikov_RPK74", "LMG RPK74"),
            new EditField_Combo.SPair("Sounds/NewWeapons/LightMachineGun/M60", "LMG M60"),
            new EditField_Combo.SPair("Sounds/NewWeapons/LightMachineGun/Pecheneg", "LMG Pecheneg"),
            new EditField_Combo.SPair("Sounds/NewWeapons/LightMachineGun/RPD", "LMG RPD"),

            new EditField_Combo.SPair("Sounds/NewWeapons/SMG/Bizon", "SMG Bizon"),
            new EditField_Combo.SPair("Sounds/NewWeapons/SMG/FN_P90", "SMG FN P90"),
            new EditField_Combo.SPair("Sounds/NewWeapons/SMG/HK_MP5SD", "SMG HK MP5SD"),
            new EditField_Combo.SPair("Sounds/NewWeapons/SMG/HK_MP5", "SMG HK MP5"),
            new EditField_Combo.SPair("Sounds/NewWeapons/SMG/HK_UMP", "SMG HK UMP"),
            new EditField_Combo.SPair("Sounds/NewWeapons/SMG/Scorpion_EVO3", "SMG Scorpion EVO3"),
            new EditField_Combo.SPair("Sounds/NewWeapons/SMG/Uzi", "SMG Uzi"),

            new EditField_Combo.SPair("Sounds/NewWeapons/Shotgun/Shotgun", "SHTG Generic"),

            new EditField_Combo.SPair("Sounds/NewWeapons/Sniper/0.308caliber", "SNP .308 Caliber"),
            new EditField_Combo.SPair("Sounds/NewWeapons/Sniper/0.50Caliber", "SNP 0.50Caliber"),
            new EditField_Combo.SPair("Sounds/NewWeapons/Sniper/7.6mmCaliber", "SNP 7.6mm Generic"),
            new EditField_Combo.SPair("Sounds/NewWeapons/Sniper/SV98", "SNP SV98"),
            new EditField_Combo.SPair("Sounds/NewWeapons/Sniper/VSS_Vintorez", "SNP VSS Vintorez"),

            new EditField_Combo.SPair("Sounds/NewWeapons/Special/40mmGrenadeLauncher", "RPG 40mm Grenade"),
            new EditField_Combo.SPair("Sounds/NewWeapons/Special/AT4RocketLauncher", "RPG AT4"),
            new EditField_Combo.SPair("Sounds/NewWeapons/Special/RPO2", "RPG RPO2"),
        };

        public static EditField_Combo.SPairs SoundReloadValues = new EditField_Combo.SPairs
        {
            new EditField_Combo.SPair("", "none"),
            new EditField_Combo.SPair("Sounds/Weapons/ASR/ASR_Generic_Reload", "ASR Generic"),
            new EditField_Combo.SPair("Sounds/Weapons/SUPPORT/SUP_Generic_Reload", "SUP Generic "),
            new EditField_Combo.SPair("Sounds/Weapons/HG/HG_Generic_Reload", "HG Generic "),
            new EditField_Combo.SPair("Sounds/Weapons/SNP/SNP_Generic_Reload", "SNP Generic "),
            new EditField_Combo.SPair("Sounds/Weapons/MG/MG_Generic_Reload", "MG Generic "),
            new EditField_Combo.SPair("Sounds/Weapons/SMG/SMG_Generic_Reload", "SMG Generic "),
            new EditField_Combo.SPair("Sounds/Weapons/SHOTGUN/SHG_Generic_Reload", "SHG Generic "),

            new EditField_Combo.SPair("Sounds/Weapons/SUPPORT/QLB_Reload", "QLB06"),
            new EditField_Combo.SPair("Sounds/Weapons/SNP/SNP_SVD_RELOAD", "SVD")
        };

        public static EditField_Combo.SPairs ScopeTypeValues = new EditField_Combo.SPairs
        {
            new EditField_Combo.SPair("default", "default"), 
            new EditField_Combo.SPair("sniper", "sniper"), 
            new EditField_Combo.SPair("cobra", "cobra"), 
            new EditField_Combo.SPair("pkasv", "pkasv"), 
            new EditField_Combo.SPair("fnfal", "fnfal"), 
            new EditField_Combo.SPair("g11", "g11"), 
            new EditField_Combo.SPair("g36", "g36"), 
            new EditField_Combo.SPair("rpg7", "rpg7"), 
            new EditField_Combo.SPair("rpg8", "rpg8"), 
            new EditField_Combo.SPair("sa80", "sa80"), 
            new EditField_Combo.SPair("svd", "svd"), 
            new EditField_Combo.SPair("kobra_red_dot", "kobra_red_dot"), 
            new EditField_Combo.SPair("acog", "acog"), 
            new EditField_Combo.SPair("pso1", "pso1"), 
            new EditField_Combo.SPair("reflex", "reflex"), 
            new EditField_Combo.SPair("fn2000", "fn2000"), 
            new EditField_Combo.SPair("L85", "L85"), 
            new EditField_Combo.SPair("xm8", "xm8"), 
            new EditField_Combo.SPair("aw50", "aw50"), 
            new EditField_Combo.SPair("dsr1", "dsr1"), 
            new EditField_Combo.SPair("psg1", "psg1"), 
            new EditField_Combo.SPair("s50hs", "s50hs"), 
            new EditField_Combo.SPair("sv98", "sv98"), 
            new EditField_Combo.SPair("wa2000", "wa2000"), 

            // fps attachment scopes
            new EditField_Combo.SPair("", ""),  // ScopeType can be empty for not-scope attachments
            new EditField_Combo.SPair("acog_fps", "acog_fps"),
            new EditField_Combo.SPair("reflex_fps", "reflex_fps"),
            new EditField_Combo.SPair("kobra_red_dot_fps", "kobra_red_dot_fps"),
            new EditField_Combo.SPair("eotech_fps", "eotech_fps"),
            new EditField_Combo.SPair("swiss_red_dot_fps", "swiss_red_dot_fps"),
       };

        public SQLField MuzzleOffset = new SQLField("MuzzleOffset");
        public SQLField MuzzleParticle = new SQLField("MuzzleParticle");
        public SQLField Animation = new SQLField("Animation");
        public SQLField BulletID = new SQLField("BulletID");
        public SQLField Sound_Shot = new SQLField("Sound_Shot");
        public SQLField Sound_Reload = new SQLField("Sound_Reload");
        public SQLField Damage = new SQLField("Damage");
        public SQLField isImmediate = new SQLField("isImmediate");
        public SQLField Mass = new SQLField("Mass");
        public SQLField Speed = new SQLField("Speed");
        public SQLField DamageDecay = new SQLField("DamageDecay");
        public SQLField Area = new SQLField("Area");
        public SQLField Delay = new SQLField("Delay");
        public SQLField Timeout = new SQLField("Timeout");
        public SQLField NumClips = new SQLField("NumClips");
        public SQLField Clipsize = new SQLField("Clipsize");
        public SQLField ReloadTime = new SQLField("ReloadTime");
        public SQLField ActiveReloadTick = new SQLField("ActiveReloadTick");
        public SQLField RateOfFire = new SQLField("RateOfFire");
        public SQLField Firemode = new SQLField("Firemode");
        public SQLField Spread = new SQLField("Spread");
        public SQLField Recoil = new SQLField("Recoil");
        public SQLField NumGrenades = new SQLField("NumGrenades");
        public SQLField GrenadeName = new SQLField("GrenadeName");
        public SQLField ScopeType = new SQLField("ScopeType");
        public SQLField ScopeZoom = new SQLField("ScopeZoom");
        public SQLField IsUpgradeable = new SQLField("IsUpgradeable");

        // FPS things
        public SQLField IsFPS = new SQLField("IsFPS");
        public SQLField FPSSpec0 = new SQLField("FPSSpec0");
        public SQLField FPSSpec1 = new SQLField("FPSSpec1");
        public SQLField FPSSpec2 = new SQLField("FPSSpec2");
        public SQLField FPSSpec3 = new SQLField("FPSSpec3");
        public SQLField FPSSpec4 = new SQLField("FPSSpec4");
        public SQLField FPSSpec5 = new SQLField("FPSSpec5");
        public SQLField FPSSpec6 = new SQLField("FPSSpec6");
        public SQLField FPSSpec7 = new SQLField("FPSSpec7");
        public SQLField FPSSpec8 = new SQLField("FPSSpec8");
        public SQLField FPSAttach0 = new SQLField("FPSAttach0");
        public SQLField FPSAttach1 = new SQLField("FPSAttach1");
        public SQLField FPSAttach2 = new SQLField("FPSAttach2");
        public SQLField FPSAttach3 = new SQLField("FPSAttach3");
        public SQLField FPSAttach4 = new SQLField("FPSAttach4");
        public SQLField FPSAttach5 = new SQLField("FPSAttach5");
        public SQLField FPSAttach6 = new SQLField("FPSAttach6");
        public SQLField FPSAttach7 = new SQLField("FPSAttach7");
        public SQLField FPSAttach8 = new SQLField("FPSAttach8");
        public SQLField AnimPrefix = new SQLField("AnimPrefix");

        public static string GET_TABLE_NAME()
        {
            return "Items_Weapons";
        }

        public string GetDisplayName()
        {
            return Name.ToString() + " (" + CategoryValues.GetNameByType(Category.ToString()) + ")";
        }

        public DBWeapon()
        {
            TABLE_ID = GET_TABLE_NAME();

            RegisterField(MuzzleOffset);
            RegisterField(MuzzleParticle);
            RegisterField(Animation);
            RegisterField(BulletID);
            RegisterField(Sound_Shot);
            RegisterField(Sound_Reload);
            RegisterField(Damage);
            RegisterField(isImmediate);
            RegisterField(Mass);
            RegisterField(Speed);
            RegisterField(DamageDecay);
            RegisterField(Area);
            RegisterField(Delay);
            RegisterField(Timeout);
            RegisterField(NumClips);
            RegisterField(Clipsize);
            RegisterField(ReloadTime);
            RegisterField(ActiveReloadTick);
            RegisterField(RateOfFire);
            RegisterField(Firemode);
            RegisterField(Spread);
            RegisterField(Recoil);
            RegisterField(NumGrenades);
            RegisterField(GrenadeName);
            RegisterField(ScopeType);
            RegisterField(ScopeZoom);
            RegisterField(IsUpgradeable);
            RegisterField(IsFPS);
            RegisterField(FPSAttach0);
            RegisterField(FPSAttach1);
            RegisterField(FPSAttach2);
            RegisterField(FPSAttach3);
            RegisterField(FPSAttach4);
            RegisterField(FPSAttach5);
            RegisterField(FPSAttach6);
            RegisterField(FPSAttach7);
            RegisterField(FPSAttach8);
            RegisterField(FPSSpec0);
            RegisterField(FPSSpec1);
            RegisterField(FPSSpec2);
            RegisterField(FPSSpec3);
            RegisterField(FPSSpec4);
            RegisterField(FPSSpec5);
            RegisterField(FPSSpec6);
            RegisterField(FPSSpec7);
            RegisterField(FPSSpec8);
            RegisterField(AnimPrefix);

            // setup varchar values
            MuzzleOffset.VARCHAR_LEN = 32;
            MuzzleParticle.VARCHAR_LEN = 32;
            Animation.VARCHAR_LEN = 32;
            BulletID.VARCHAR_LEN = 32;
            GrenadeName.VARCHAR_LEN = 32;
            ScopeType.VARCHAR_LEN = 32;
            AnimPrefix.VARCHAR_LEN = 32;
        }

        public void SetNewRecordDefaults(int category)
        {
            SetNewItemDefaults(category);

            MuzzleOffset.FromObject("0 0 0");
            MuzzleParticle.FromObject("muzzle_asr");
            Animation.FromObject("assault");
            BulletID.FromObject("5.45");
            Sound_Shot.FromObject("Sounds/Weapons/AK74_7_62_Shot");
            Sound_Reload.FromObject("Sounds/Weapons/AK74_Reload");
            Damage.FromObject(20.0f);
            isImmediate.FromObject(1);
            Mass.FromObject(10.0f);
            Speed.FromObject(300.0f);
            DamageDecay.FromObject(50.0f);
            Area.FromObject(0.0f);
            Delay.FromObject(0.0f);
            Timeout.FromObject(0.0f);
            NumClips.FromObject(10);
            Clipsize.FromObject(30);
            ReloadTime.FromObject(4.0f);
            ActiveReloadTick.FromObject(2.0f);
            RateOfFire.FromObject(10);
            Firemode.FromObject("100");
            Spread.FromObject(20.0f);
            Recoil.FromObject(20.0f);
            NumGrenades.FromObject(0);
            GrenadeName.FromObject("asr_grenade");
            ScopeType.FromObject("default");
            ScopeZoom.FromObject(0);
            IsUpgradeable.FromObject(1);
            IsFPS.FromObject(0);
            FPSSpec0.FromObject(0);
            FPSSpec1.FromObject(0);
            FPSSpec2.FromObject(0);
            FPSSpec3.FromObject(0);
            FPSSpec4.FromObject(0);
            FPSSpec5.FromObject(0);
            FPSSpec6.FromObject(0);
            FPSSpec7.FromObject(0);
            FPSSpec8.FromObject(0);
            FPSAttach0.FromObject(0);
            FPSAttach1.FromObject(0);
            FPSAttach2.FromObject(0);
            FPSAttach3.FromObject(0);
            FPSAttach4.FromObject(0);
            FPSAttach5.FromObject(0);
            FPSAttach6.FromObject(0);
            FPSAttach7.FromObject(0);
            FPSAttach8.FromObject(0);
            AnimPrefix.FromObject("");

            if (category == 28)
            {
            }

        }
    }
}
