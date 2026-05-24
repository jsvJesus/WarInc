using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace WOAdmin
{
    public partial class FormAddPackage : EditRecord
    {
        DBPackage rec_ = new DBPackage();

        public FormAddPackage(int category)
        {
            InitializeComponent();

            Reg("ItemID", SQLDataType.INT).RO(true);

            Reg("FNAME", SQLDataType.STRING);
            Reg("Name", SQLDataType.STRING);
            EditField efdesc = Reg("Description", SQLDataType.STRING);
            Reg("Category", SQLDataType.STRING, new EditField_Combo(DBPackage.CategoryValues)).RO(true);
            Reg("LevelRequired", SQLDataType.INT);

            (efdesc.edit as TextBox).Multiline = true;
            efdesc.edit.Size = new Size(efdesc.edit.Width, efdesc.temp_oldheight);

            // set default vars
            rec_.SetNewRecordDefaults(category);
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
            Form1.packages_.TEMP_ScanForNewInDB();

            Close();
        }
    }
}
