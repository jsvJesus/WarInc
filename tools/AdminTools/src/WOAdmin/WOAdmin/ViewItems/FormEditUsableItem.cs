using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace WOAdmin
{
    public partial class FormEditUsableItem : EditRecord
    {
        public FormEditUsableItem()
        {
            InitializeComponent();

            Reg("ItemID", SQLDataType.INT);
            Reg("FNAME", SQLDataType.STRING);
            Reg("Category", SQLDataType.STRING, new EditField_Combo(DBWeapon.CategoryValues)).RO(true);
            Reg("Name", SQLDataType.STRING);
            EditField efdesc = Reg("Description", SQLDataType.STRING);
            Reg("BulletID", SQLDataType.STRING, new EditField_Combo(DBWeapon.BulletIDValues));
            Reg("Sound_Shot", SQLDataType.STRING, new EditField_Combo(DBWeapon.SoundShotValues));
            Reg("Sound_Reload", SQLDataType.STRING, new EditField_Combo(DBWeapon.SoundReloadValues));
            Reg("NumClips", SQLDataType.INT);
            Reg("Clipsize", SQLDataType.INT);
            Reg("ScopeType", SQLDataType.STRING, new EditField_Combo(DBWeapon.ScopeTypeValues));
            Reg("ScopeZoom", SQLDataType.INT);
            Reg("LevelRequired", SQLDataType.INT);
            Reg("RateOfFire", SQLDataType.INT);
            Reg("Damage", SQLDataType.DOUBLE);

            Reg("IsFPS", SQLDataType.INT);
            Reg("AnimPrefix", SQLDataType.STRING);

            Reg("MuzzleParticle", SQLDataType.STRING, new EditField_Combo(DBWeapon.MuzzleParticleValues));
            Reg("Mass", SQLDataType.DOUBLE);
            Reg("Speed", SQLDataType.DOUBLE);
            Reg("DamageDecay", SQLDataType.DOUBLE);
            Reg("Area", SQLDataType.DOUBLE);
            Reg("Delay", SQLDataType.DOUBLE);
            Reg("Timeout", SQLDataType.DOUBLE);
            Reg("ReloadTime", SQLDataType.DOUBLE);
            Reg("ActiveReloadTick", SQLDataType.DOUBLE);
            Reg("Spread", SQLDataType.DOUBLE);
            Reg("Recoil", SQLDataType.DOUBLE);
            Reg("NumGrenades", SQLDataType.INT);

            (efdesc.edit as TextBox).Multiline = true;
            efdesc.edit.Size = new Size(efdesc.edit.Width, efdesc.temp_oldheight);
        }
    }
}
