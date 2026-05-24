using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace WOAdmin
{
    public partial class FormEditWeapon : EditRecord
    {
        public FormEditWeapon()
        {
            InitializeComponent();

            Reg("ItemID", SQLDataType.INT);
            Reg("FNAME", SQLDataType.STRING);
            Reg("Category", SQLDataType.STRING, new EditField_Combo(DBWeapon.CategoryValues)).RO(true);
            Reg("Name", SQLDataType.STRING);
            EditField efdesc = Reg("Description", SQLDataType.STRING);
            Reg("MuzzleOffset", SQLDataType.STRING);
            Reg("MuzzleParticle", SQLDataType.STRING, new EditField_Combo(DBWeapon.MuzzleParticleValues));
            Reg("Animation", SQLDataType.STRING, new EditField_Combo(DBWeapon.AnimationValues));
            Reg("BulletID", SQLDataType.STRING, new EditField_Combo(DBWeapon.BulletIDValues));
            Reg("Sound_Shot", SQLDataType.STRING, new EditField_Combo(DBWeapon.SoundShotValues));
            Reg("Sound_Reload", SQLDataType.STRING, new EditField_Combo(DBWeapon.SoundReloadValues));
            Reg("Damage", SQLDataType.DOUBLE);
            Reg("isImmediate", SQLDataType.INT);
            Reg("Mass", SQLDataType.DOUBLE);
            Reg("Speed", SQLDataType.DOUBLE);
            Reg("DamageDecay", SQLDataType.DOUBLE);
            Reg("Area", SQLDataType.DOUBLE);
            Reg("Delay", SQLDataType.DOUBLE);
            Reg("Timeout", SQLDataType.DOUBLE);
            Reg("NumClips", SQLDataType.INT);
            Reg("Clipsize", SQLDataType.INT);
            Reg("ReloadTime", SQLDataType.DOUBLE);
            Reg("ActiveReloadTick", SQLDataType.DOUBLE);
            Reg("RateOfFire", SQLDataType.INT);
            Reg("Firemode", SQLDataType.STRING, new EditField_Combo(DBWeapon.FiremodeValues));
            Reg("Spread", SQLDataType.DOUBLE);
            Reg("Recoil", SQLDataType.DOUBLE);
            Reg("NumGrenades", SQLDataType.INT);
            Reg("GrenadeName", SQLDataType.STRING, new EditField_Combo(DBWeapon.GrenadeNameValues));
            Reg("ScopeType", SQLDataType.STRING, new EditField_Combo(DBWeapon.ScopeTypeValues));
            Reg("ScopeZoom", SQLDataType.INT);
            Reg("LevelRequired", SQLDataType.INT);
            Reg("IsUpgradeable", SQLDataType.INT);

            Reg("IsFPS", SQLDataType.INT);
            Reg("FPSSpec0", SQLDataType.INT, "Muzzle");
            Reg("FPSSpec1", SQLDataType.INT, "Upper Rail");
            Reg("FPSSpec2", SQLDataType.INT, "Left Rail");
            Reg("FPSSpec3", SQLDataType.INT, "Bottom Rail");
            Reg("FPSSpec4", SQLDataType.INT, "Clip");
            Reg("FPSSpec5", SQLDataType.INT, "Receiver");
            Reg("FPSSpec6", SQLDataType.INT, "Stock");
            Reg("FPSSpec7", SQLDataType.INT, "Barrel");
            Reg("FPSSpec8", SQLDataType.INT, "Paint");
            Reg("FPSAttach0", SQLDataType.INT, "default");
            Reg("FPSAttach1", SQLDataType.INT, "default");
            Reg("FPSAttach2", SQLDataType.INT, "default");
            Reg("FPSAttach3", SQLDataType.INT, "default");
            Reg("FPSAttach4", SQLDataType.INT, "default");
            Reg("FPSAttach5", SQLDataType.INT, "default");
            Reg("FPSAttach6", SQLDataType.INT, "default");
            Reg("FPSAttach7", SQLDataType.INT, "default");
            Reg("FPSAttach8", SQLDataType.INT, "default");
            Reg("AnimPrefix", SQLDataType.STRING);

            (efdesc.edit as TextBox).Multiline = true;
            efdesc.edit.Size = new Size(efdesc.edit.Width, efdesc.temp_oldheight);
        }
    }
}
