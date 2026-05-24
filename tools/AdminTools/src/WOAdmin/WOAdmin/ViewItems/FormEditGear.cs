using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace WOAdmin
{
    public partial class FormEditGear : EditRecord
    {
        public FormEditGear()
        {
            InitializeComponent();

            Reg("ItemID", SQLDataType.INT);
            Reg("FNAME", SQLDataType.STRING);
            Reg("Name", SQLDataType.STRING);
            EditField efdesc = Reg("Description", SQLDataType.STRING);
            Reg("Category", SQLDataType.STRING, new EditField_Combo(DBGear.CategoryValues)).RO(true);
            Reg("Weight", SQLDataType.INT);
            Reg("DamagePerc", SQLDataType.INT);
            Reg("DamageMax", SQLDataType.INT);
            Reg("Bulkiness", SQLDataType.INT);
            Reg("Inaccuracy", SQLDataType.INT);
            Reg("Stealth", SQLDataType.INT);
            Reg("Protection", SQLDataType.STRING);
            Reg("CustomFunction", SQLDataType.INT);
            Reg("ProtectionLevel", SQLDataType.INT);
            Reg("LevelRequired", SQLDataType.INT);

            (efdesc.edit as TextBox).Multiline = true;
            efdesc.edit.Size = new Size(efdesc.edit.Width, efdesc.temp_oldheight);
        }
    }
}
