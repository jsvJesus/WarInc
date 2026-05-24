using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace WOAdmin
{
    public partial class FormEditPackage : EditRecord
    {
        public FormEditPackage()
        {
            InitializeComponent();

            Reg("ItemID", SQLDataType.INT);
            Reg("FNAME", SQLDataType.STRING);
            Reg("Name", SQLDataType.STRING);
            EditField efdesc = Reg("Description", SQLDataType.STRING);
            Reg("LevelRequired", SQLDataType.INT);
            Reg("IsEnabled", SQLDataType.INT);

            Reg("AddGP", SQLDataType.INT, "+War Points");
            Reg("AddSP", SQLDataType.INT, "+Skill Points");
            Reg("Item1_ID", SQLDataType.INT);
            Reg("Item1_Exp", SQLDataType.INT);
            Reg("Item2_ID", SQLDataType.INT);
            Reg("Item2_Exp", SQLDataType.INT);
            Reg("Item3_ID", SQLDataType.INT);
            Reg("Item3_Exp", SQLDataType.INT);
            Reg("Item4_ID", SQLDataType.INT);
            Reg("Item4_Exp", SQLDataType.INT);
            Reg("Item5_ID", SQLDataType.INT);
            Reg("Item5_Exp", SQLDataType.INT);
            Reg("Item6_ID", SQLDataType.INT);
            Reg("Item6_Exp", SQLDataType.INT);

            (efdesc.edit as TextBox).Multiline = true;
            efdesc.edit.Size = new Size(efdesc.edit.Width, efdesc.temp_oldheight);
        }
    }
}
