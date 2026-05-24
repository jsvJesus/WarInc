using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace WOAdmin
{
    public partial class FormEditUser : EditRecord
    {
        EditField ef_email_;
        EditField ef_admin_note_;

        EditField ef_st1_;
        EditField ef_st2_;
        EditField ef_st3_;
        EditField ef_st4_;
        EditField ef_st5_;
        EditField ef_st6_;
        EditField ef_st7_;
        EditField ef_st8_;
        EditField ef_st9_;
        EditField ef_stA_;

        public FormEditUser()
        {
            InitializeComponent();

            Reg("CustomerID", SQLDataType.INT);
            Reg("AccountName", SQLDataType.STRING);
            Reg("AccountStatus", SQLDataType.STRING, new EditField_Combo(DBUser.AccountStatusValues));
            Reg("GamePoints", SQLDataType.INT);
            Reg("GameDollars", SQLDataType.INT);
            Reg("HonorPoints", SQLDataType.INT);
            Reg("SkillPoints", SQLDataType.INT);
            Reg("Gamertag", SQLDataType.STRING);
            Reg("dateregistered", SQLDataType.DATETIME).RO(true);
            Reg("lastlogindate", SQLDataType.DATETIME);
            Reg("lastloginIP", SQLDataType.STRING);
            Reg("lastgamedate", SQLDataType.DATETIME);
            Reg("ReferralID", SQLDataType.INT).RO(true);
            Reg("IsFPSEnabled", SQLDataType.INT);
            Reg("IsDeveloper", SQLDataType.INT);

            ef_email_ = Reg("AccountInfo:email", SQLDataType.STRING);
            ef_admin_note_ = Reg("AccountInfo:admin_note", SQLDataType.STRING);

            ef_st1_ = Reg("ST:Kills", SQLDataType.INT).RO(true);
            ef_st2_ = Reg("ST:Deaths", SQLDataType.INT).RO(true);
            ef_st3_ = Reg("ST:ShotsFired", SQLDataType.INT).RO(true);
            ef_st4_ = Reg("ST:ShotsHits", SQLDataType.INT).RO(true);
            ef_st5_ = Reg("ST:Headshots", SQLDataType.INT).RO(true);
            ef_st6_ = Reg("ST:Wins", SQLDataType.INT).RO(true);
            ef_st7_ = Reg("ST:Losses", SQLDataType.INT).RO(true);
            ef_st8_ = Reg("ST:CapNeutral", SQLDataType.INT).RO(true);
            ef_st9_ = Reg("ST:CapEnemy", SQLDataType.INT).RO(true);
            ef_stA_ = Reg("ST:TimePlayed", SQLDataType.INT).RO(true);
        }

        override public void SetFromRecord(SQLRec rec)
        {
            base.SetFromRecord(rec);

            try
            {
                ef_email_.edit.Text = (rec as DBUser).refl_AccountInfo.email.ToString();
                ef_email_.edit.Enabled = true;

                ef_admin_note_.edit.Text = (rec as DBUser).refl_AccountInfo.admin_note.ToString();
                ef_admin_note_.edit.Enabled = true;
            }
            catch { }

            try
            {
                DBStatsRO stat = (rec as DBUser).refl_Stats;
                ef_st1_.edit.Text = stat.Kills.ToString();
                ef_st2_.edit.Text = stat.Deaths.ToString();
                ef_st3_.edit.Text = stat.ShotsFired.ToString();
                ef_st4_.edit.Text = stat.ShotsHits.ToString();
                ef_st5_.edit.Text = stat.Headshots.ToString();
                ef_st6_.edit.Text = stat.Wins.ToString();
                ef_st7_.edit.Text = stat.Losses.ToString();
                ef_st8_.edit.Text = stat.CaptureNeutralPoints.ToString();
                ef_st9_.edit.Text = stat.CaptureEnemyPoints.ToString();
                ef_stA_.edit.Text = stat.TimePlayed.ToString();
            }
            catch { }
        }

        override public bool ERSave()
        {
            if (base.ERSave() == false)
                return false;

            DBAccountInfo accInfo = (sqlrec_ as DBUser).refl_AccountInfo;
            accInfo.email.v_string = ef_email_.edit.Text;
            accInfo.admin_note.v_string = ef_admin_note_.edit.Text;
            Form1.sql.Update(accInfo);
            return true;
        }
    }
}
