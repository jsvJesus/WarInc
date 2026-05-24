using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace WOAdmin
{
    public partial class FormEditClan : EditRecord
    {
        public FormEditClan()
        {
            InitializeComponent();

            Reg("ClanID", SQLDataType.INT);
            Reg("OwnerID", SQLDataType.INT).RO(true);
            Reg("ClanName", SQLDataType.STRING);
            Reg("ClanTag", SQLDataType.STRING);
            EditField eflore = Reg("ClanLore", SQLDataType.STRING);
            Reg("NumClanMembers", SQLDataType.INT).RO(true);
            Reg("MaxClanMembers", SQLDataType.INT);
            Reg("ClanXP", SQLDataType.INT);
            Reg("ClanGP", SQLDataType.INT);
            Reg("ClanCreateDate", SQLDataType.DATETIME).RO(true);

            (eflore.edit as TextBox).Multiline = true;
            eflore.edit.Size = new Size(eflore.edit.Width, eflore.temp_oldheight);
        }
    }
}
