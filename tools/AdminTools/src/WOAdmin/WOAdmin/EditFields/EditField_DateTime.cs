using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;

namespace WOAdmin
{
    public partial class EditField_DateTime : DateTimePicker, IEditField
    {
        public EditField_DateTime()
        {
            this.Format = DateTimePickerFormat.Custom;
            this.CustomFormat = "MM/dd/yyyy HH:mm";
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
            if (fld.type2 != SQLDataType.DATETIME)
                throw new ArgumentException("EditField_DateTime used not in time");
            this.Value = fld.v_time;
        }

        void IEditField.FldGet(SQLField fld)
        {
            fld.type2 = SQLDataType.DATETIME;
            fld.v_time = this.Value;
        }
    }
}
