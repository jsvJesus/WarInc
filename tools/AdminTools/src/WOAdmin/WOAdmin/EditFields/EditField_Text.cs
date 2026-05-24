using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;

namespace WOAdmin
{
    public partial class EditField_Text : TextBox, IEditField
    {
        public EditField_Text()
        {
        }

        bool IEditField.FldIsValid(ref string msg)
        {
            return true;
        }

        void IEditField.FldReset()
        {
            this.Text = "";
        }

        void IEditField.FldSet(SQLField fld)
        {
            this.Text = fld.ToString();
        }

        void IEditField.FldGet(SQLField fld)
        {
            fld.type2 = SQLDataType.STRING;
            fld.v_string = this.Text;
        }
    }
}
