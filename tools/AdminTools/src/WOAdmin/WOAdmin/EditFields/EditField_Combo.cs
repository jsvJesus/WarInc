using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;

namespace WOAdmin
{
    public partial class EditField_Combo : ComboBox, IEditField
    {
        public class SPair
        {
            public string type;
            public string name;
            public SPair(string s1, string s2)
            {
                type = s1;
                name = s2;
            }
        };
        public class SPairs : List<SPair>
        {
            public string GetNameByType(string type)
            {
                for (int i = 0; i < this.Count; i++)
                {
                    if (type.Equals(this[i].type) == true)
                    {
                        return this[i].name;
                    }
                }
                return "-UNKNOWN-";
            }
        };
        SPairs pairs;

        public EditField_Combo(SPairs in_pairs)
        {
            pairs = in_pairs;

            this.Sorted = false;
            this.DropDownStyle = ComboBoxStyle.DropDownList;
            this.Items.Add("-INVALID-");
            foreach (SPair p in pairs)
            {
                this.Items.Add(p.name);
            }

            this.SelectedIndex = 0;
        }

        bool IEditField.FldIsValid(ref string msg)
        {
            if (this.SelectedIndex == 0)
            {
                msg = "Invalid selection";
                return false;
            }
            return true;
        }

        void IEditField.FldReset()
        {
            this.SelectedIndex = 0;
        }

        void IEditField.FldSet(SQLField fld)
        {
            this.SelectedIndex = 0; // "invalid entry" by default

            string val = fld.ToString();
            for (int i = 0; i < pairs.Count; i++)
            {
                if (val.Equals(pairs[i].type) == true)
                {
                    this.SelectedIndex = i + 1;
                    return;
                }
            }
        }

        void IEditField.FldGet(SQLField fld)
        {
            fld.type2 = SQLDataType.STRING;
            if (this.SelectedIndex == 0)
            {
                fld.v_string = pairs[0].type;
                return;
            }

            fld.v_string = pairs[this.SelectedIndex - 1].type;
        }
    }
}
