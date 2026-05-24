using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Data.SqlClient;

namespace WOAdmin
{
    /*
    CouponId int Unchecked
    CouponNumber	varchar(64)	Unchecked
    RemainingUses	int	Unchecked
    ActionCode	varchar(16)	Unchecked
    * * */

    public class DBCoupon : SQLRec
    {
        public SQLField CouponNumber = new SQLField("CouponNumber");
        public SQLField RemainingUses = new SQLField("RemainingUses");
        public SQLField ActionCode = new SQLField("ActionCode");

        public static string GET_TABLE_NAME()
        {
            return "Coupons";
        }

        public DBCoupon()
        {
            TABLE_ID = GET_TABLE_NAME();
            RECORD_ID = new SQLField("CouponId");

            RegisterField(CouponNumber);
            RegisterField(RemainingUses);
            RegisterField(ActionCode);

            // setup varchar values
            CouponNumber.VARCHAR_LEN = 64;
            ActionCode.VARCHAR_LEN = 16;
        }
    }
}
