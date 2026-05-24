namespace WOAdmin
{
    partial class ViewSecurityLog
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.dtpickFrom = new System.Windows.Forms.DateTimePicker();
            this.label4 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.dtpickTo = new System.Windows.Forms.DateTimePicker();
            this.listViewR = new System.Windows.Forms.ListView();
            this.btnUpdate = new System.Windows.Forms.Button();
            this.columnHeader7 = new System.Windows.Forms.ColumnHeader();
            this.SuspendLayout();
            // 
            // dtpickFrom
            // 
            this.dtpickFrom.CustomFormat = "";
            this.dtpickFrom.Format = System.Windows.Forms.DateTimePickerFormat.Short;
            this.dtpickFrom.Location = new System.Drawing.Point(93, 15);
            this.dtpickFrom.Name = "dtpickFrom";
            this.dtpickFrom.Size = new System.Drawing.Size(143, 20);
            this.dtpickFrom.TabIndex = 4;
            this.dtpickFrom.Value = new System.DateTime(2010, 1, 1, 0, 0, 0, 0);
            // 
            // label4
            // 
            this.label4.Location = new System.Drawing.Point(12, 18);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(78, 13);
            this.label4.TabIndex = 9;
            this.label4.Text = "From:";
            this.label4.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // label5
            // 
            this.label5.Location = new System.Drawing.Point(12, 44);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(78, 13);
            this.label5.TabIndex = 13;
            this.label5.Text = "To:";
            this.label5.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // dtpickTo
            // 
            this.dtpickTo.CustomFormat = "";
            this.dtpickTo.Format = System.Windows.Forms.DateTimePickerFormat.Short;
            this.dtpickTo.Location = new System.Drawing.Point(93, 41);
            this.dtpickTo.Name = "dtpickTo";
            this.dtpickTo.Size = new System.Drawing.Size(143, 20);
            this.dtpickTo.TabIndex = 16;
            this.dtpickTo.Value = new System.DateTime(2014, 1, 1, 0, 0, 0, 0);
            // 
            // listViewR
            // 
            this.listViewR.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.listViewR.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader7});
            this.listViewR.FullRowSelect = true;
            this.listViewR.GridLines = true;
            this.listViewR.Location = new System.Drawing.Point(1, 80);
            this.listViewR.Name = "listViewR";
            this.listViewR.Size = new System.Drawing.Size(740, 70);
            this.listViewR.TabIndex = 17;
            this.listViewR.UseCompatibleStateImageBehavior = false;
            this.listViewR.View = System.Windows.Forms.View.Details;
            // 
            // btnUpdate
            // 
            this.btnUpdate.Location = new System.Drawing.Point(273, 15);
            this.btnUpdate.Name = "btnUpdate";
            this.btnUpdate.Size = new System.Drawing.Size(69, 53);
            this.btnUpdate.TabIndex = 18;
            this.btnUpdate.Text = "Update";
            this.btnUpdate.UseVisualStyleBackColor = true;
            this.btnUpdate.Click += new System.EventHandler(this.btnUpdate_Click);
            // 
            // ViewSecurityLog
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(745, 461);
            this.Controls.Add(this.btnUpdate);
            this.Controls.Add(this.listViewR);
            this.Controls.Add(this.dtpickTo);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.dtpickFrom);
            this.Name = "ViewSecurityLog";
            this.Text = "ViewTransactions";
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.DateTimePicker dtpickFrom;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.DateTimePicker dtpickTo;
        private System.Windows.Forms.ListView listViewR;
        private System.Windows.Forms.Button btnUpdate;
        private System.Windows.Forms.ColumnHeader columnHeader7;

    }
}