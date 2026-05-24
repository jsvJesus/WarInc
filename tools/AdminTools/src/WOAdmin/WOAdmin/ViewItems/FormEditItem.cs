using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace WOAdmin
{
    public partial class FormEditItem : EditRecord
    {
        public FormEditItem()
        {
            InitializeComponent();

            Reg("ItemID", SQLDataType.INT);
            Reg("FNAME", SQLDataType.STRING);
            Reg("Name", SQLDataType.STRING);
            EditField efdesc = Reg("Description", SQLDataType.STRING);
            Reg("Category", SQLDataType.STRING, new EditField_Combo(DBItem.CategoryValues)).RO(true);
            Reg("LevelRequired", SQLDataType.INT);
            Reg("IsStackable", SQLDataType.INT);

            (efdesc.edit as TextBox).Multiline = true;
            efdesc.edit.Size = new Size(efdesc.edit.Width, efdesc.temp_oldheight);
        }
    }
}
