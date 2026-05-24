using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;

namespace WOAdmin
{
    public class WOClans : UIDBListView<DBClanData>
    {
        public WOClans()
        {
        }

        override protected string GET_TABLE_NAME()
        {
            return DBClanData.GET_TABLE_NAME();
        }

        override protected void CreateListViewColumns()
        {
            listView.Columns.Clear();
            listView.Columns.Add("Name", 200);
            listView.Columns.Add("Tag", 80);
            listView.Columns.Add("Cur", 50);
            listView.Columns.Add("Max", 50);
            listView.Columns.Add("GP", 80);
            listView.Columns.Add("XP", 80);
            listView.Columns.Add("Level", 45);
            listView.Columns.Add("Lore", 500);
        }

        override public void UpdateListView(SQLRec in_rec)
        {
            DBClanData rec = in_rec as DBClanData;
            System.Diagnostics.Debug.Assert(rec != null);
            ListViewItem lvi = rec.ui_lvi;
            System.Diagnostics.Debug.Assert(lvi != null);

            lvi.SubItems.Clear();
            lvi.UseItemStyleForSubItems = false;
            lvi.Text = rec.ClanName.ToString();
            lvi.SubItems.Add(rec.ClanTag.ToString());
            lvi.SubItems.Add(rec.NumClanMembers.ToString());
            lvi.SubItems.Add(rec.MaxClanMembers.ToString());
            lvi.SubItems.Add(rec.ClanGP.ToString());
            lvi.SubItems.Add(rec.ClanXP.ToString());
            lvi.SubItems.Add(rec.ClanLevel.ToString());
            lvi.SubItems.Add(rec.ClanLore.ToString());
        }
    }
}
