using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace WOAdmin
{
    public partial class FormAddWeapon : EditRecord
    {
        DBWeapon rec_ = new DBWeapon();

        public FormAddWeapon(int category)
        {
            InitializeComponent();

            Reg("ItemID", SQLDataType.INT).RO(true);

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

            (efdesc.edit as TextBox).Multiline = true;
            efdesc.edit.Size = new Size(efdesc.edit.Width, efdesc.temp_oldheight);

            // set default vars
            rec_.SetNewRecordDefaults(category);
            SetFromRecord(rec_);
        }

        bool ValidateInput()
        {
            //@TODO
            return true;
        }

        private void btnAdd_Click(object sender, EventArgs e)
        {
            if (!ValidateEdit())
                return;

            FinishEdit();

            if (!ValidateInput())
            {
                MessageBox.Show("Missing input data", "");
                return;
            }

            Form1.sql.InsertNew(rec_);
            Form1.weapons_.TEMP_ScanForNewInDB();

            Close();
        }
    }
}
