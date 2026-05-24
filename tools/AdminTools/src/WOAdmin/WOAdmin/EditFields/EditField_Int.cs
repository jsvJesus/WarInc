using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;

namespace WOAdmin
{
    public partial class EditField_Int : TextBox, IEditField
    {
        Color oldBgClr = Color.White;
        int min_ = Int32.MinValue;
        int max_ = Int32.MaxValue;
        public EditField_Int()
        {
            oldBgClr = this.BackColor;
            //this.BackColor = Color.Green;
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
                int val = Convert.ToInt32(this.Text);
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
                msg = string.Format("'{0}' is not a integer", this.Text);
                this.BackColor = Color.Red;
                return false;
            }

            if(this.BackColor != oldBgClr) 
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
            fld.type2 = SQLDataType.INT;
            fld.v_int = Convert.ToInt32(this.Text);
        }
    }
}
