using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace WOAdmin
{
    public partial class FormEditAttachment : EditRecord
    {
        public FormEditAttachment()
        {
            InitializeComponent();

            Reg("ItemID", SQLDataType.INT);
            Reg("FNAME", SQLDataType.STRING);
            Reg("Name", SQLDataType.STRING);
            EditField efdesc = Reg("Description", SQLDataType.STRING);
            Reg("Category", SQLDataType.STRING, new EditField_Combo(DBAttachment.CategoryValues)).RO(true);
            Reg("LevelRequired", SQLDataType.INT);
            Reg("Type", SQLDataType.STRING, new EditField_Combo(DBAttachment.TypeValues));

            Reg("SpecID", SQLDataType.INT);
            Reg("Damage", SQLDataType.DOUBLE);
            Reg("Range", SQLDataType.DOUBLE);
            Reg("Firerate", SQLDataType.DOUBLE);
            Reg("Recoil", SQLDataType.DOUBLE);
            Reg("Spread", SQLDataType.DOUBLE);
            Reg("Clipsize", SQLDataType.INT);
            Reg("Scopemag", SQLDataType.DOUBLE);
            Reg("Scopetype", SQLDataType.STRING, new EditField_Combo(DBWeapon.ScopeTypeValues));
            Reg("AnimPrefix", SQLDataType.STRING);
            Reg("MuzzleParticle", SQLDataType.STRING);
            Reg("FireSound", SQLDataType.STRING, new EditField_Combo(DBWeapon.SoundShotValues));

            (efdesc.edit as TextBox).Multiline = true;
            efdesc.edit.Size = new Size(efdesc.edit.Width, efdesc.temp_oldheight);
        }
    }
}
