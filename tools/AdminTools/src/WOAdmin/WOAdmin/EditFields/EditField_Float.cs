using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;

namespace WOAdmin
{
    public partial class EditField_Float : TextBox, IEditField
    {
        Color oldBgClr = Color.White;
        float min_ = -99999;
        float max_ = 99999;
        public EditField_Float()
        {
            oldBgClr = this.BackColor;
            //this.BackColor = Color.Blue;
            this.TextChanged += new System.EventHandler(this.OnTextChanged);
        }

        private void OnTextChanged(object sender, EventArgs e)
        {
            string msg = "";
            (this as IEditField).FldIsValid(ref msg);
        }

        bool IEditField.FldIsValid(ref string msg)
        {
            try
            {
                double val = Convert.ToDouble(this.Text);
                if (val < min_ || val >= max_)
                {
                    msg = string.Format("Value {2} must be in range {0}-{1}", 
                        min_, max_, val);
                    this.BackColor = Color.Red;
                    return false;
                }
            }
            catch
            {
                msg = string.Format("'{0}' is not a floating point number", this.Text);
                this.BackColor = Color.Red;
                return false;
            }

            if (this.BackColor != oldBgClr)
                this.BackColor = oldBgClr;
            return true;
        }

        void IEditField.FldReset()
        {
            this.Text = "0";
        }

        void IEditField.FldSet(SQLField fld)
        {
            this.Text = fld.ToString();
        }

        void IEditField.FldGet(SQLField fld)
        {
            fld.type2 = SQLDataType.DOUBLE;
            fld.v_double = Convert.ToDouble(this.Text);
        }
    }
}
