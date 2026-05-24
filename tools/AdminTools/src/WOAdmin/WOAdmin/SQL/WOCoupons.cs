using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;

namespace WOAdmin
{
    public class WOCoupons : UIDBListView<DBCoupon>
    {
        public WOCoupons()
        {
        }

        override protected string GET_TABLE_NAME()
        {
            return DBCoupon.GET_TABLE_NAME();
        }

        override protected void CreateListViewColumns()
        {
            listView.Columns.Clear();
            listView.Columns.Add("CouponNumber", 200);
            listView.Columns.Add("Remaining", 100);
            listView.Columns.Add("ActionCode", 100);
        }

        override public void UpdateListView(SQLRec in_rec)
        {
            ListViewItem lvi = in_rec.ui_lvi;
            System.Diagnostics.Debug.Assert(lvi != null);
            DBCoupon rec = in_rec as DBCoupon;
            System.Diagnostics.Debug.Assert(rec != null);

            lvi.SubItems.Clear();
            lvi.Text = rec.CouponNumber.ToString();
            lvi.SubItems.Add(rec.RemainingUses.ToString());
            lvi.SubItems.Add(rec.ActionCode.ToString());
        }
    }
}
