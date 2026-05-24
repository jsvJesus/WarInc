using System;
using System.Collections.Generic;
using System.Text;

namespace WOAdmin
{
    interface IAdminView
    {
        void OnViewActivated();
        void OnViewDeactivated();
    }
}
