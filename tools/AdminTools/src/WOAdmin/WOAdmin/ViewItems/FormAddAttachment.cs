using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace WOAdmin
{
    public partial class FormAddAttachment : EditRecord
    {
        DBAttachment rec_ = new DBAttachment();

        public FormAddAttachment(int category)
        {
            InitializeComponent();

            Reg("ItemID", SQLDataType.INT).RO(true);

            Reg("FNAME", SQLDataType.STRING);
            Reg("Name", SQLDataType.STRING);
            EditField efdesc = Reg("Description", SQLDataType.STRING);

            (efdesc.edit as TextBox).Multiline = true;
            efdesc.edit.Size = new Size(efdesc.edit.Width, efdesc.temp_oldheight);

            // set default vars
            rec_.SetNewRecordDefaults();
            SetFromRecord(rec_);
        }

        bool ValidateInput()
        {
            //@TODO
            return true;
        }

        private void btnAdd_Click(object sender, EventArgs e)
        {
            if (!ValidateEdit())
                return;

            FinishEdit();

            if (!ValidateInput())
            {
                MessageBox.Show("Missing input data", "");
                return;
            }

            Form1.sql.InsertNew(rec_);
            Form1.attachments_.TEMP_ScanForNewInDB();

            Close();
        }
    }
}
