using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;

namespace WOAdmin
{
    public interface IEditField
    {
        void FldReset();
        bool FldIsValid(ref string msg);
        void FldSet(SQLField fld);
        void FldGet(SQLField fld);
    };
}
