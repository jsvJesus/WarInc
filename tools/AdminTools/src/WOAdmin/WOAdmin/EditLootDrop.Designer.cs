namespace WOAdmin
{
    partial class EditLootDrop
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
            this.panel1 = new System.Windows.Forms.Panel();
            this.lblLootId = new System.Windows.Forms.Label();
            this.btnSave = new System.Windows.Forms.Button();
            this.lblBoxInfo = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // panel1
            // 
            this.panel1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.panel1.AutoScroll = true;
            this.panel1.Location = new System.Drawing.Point(12, 48);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(820, 608);
            this.panel1.TabIndex = 0;
            // 
            // lblLootId
            // 
            this.lblLootId.AutoSize = true;
            this.lblLootId.Location = new System.Drawing.Point(12, 9);
            this.lblLootId.Name = "lblLootId";
            this.lblLootId.Size = new System.Drawing.Size(35, 13);
            this.lblLootId.TabIndex = 1;
            this.lblLootId.Text = "label1";
            // 
            // btnSave
            // 
            this.btnSave.Location = new System.Drawing.Point(671, 9);
            this.btnSave.Name = "btnSave";
            this.btnSave.Size = new System.Drawing.Size(75, 30);
            this.btnSave.TabIndex = 2;
            this.btnSave.Text = "Save";
            this.btnSave.UseVisualStyleBackColor = true;
            this.btnSave.Click += new System.EventHandler(this.btnSave_Click);
            // 
            // lblBoxInfo
            // 
            this.lblBoxInfo.AutoSize = true;
            this.lblBoxInfo.Location = new System.Drawing.Point(197, 9);
            this.lblBoxInfo.Name = "lblBoxInfo";
            this.lblBoxInfo.Size = new System.Drawing.Size(35, 13);
            this.lblBoxInfo.TabIndex = 3;
            this.lblBoxInfo.Text = "label1";
            // 
            // EditLootDrop
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(844, 668);
            this.Controls.Add(this.lblBoxInfo);
            this.Controls.Add(this.btnSave);
            this.Controls.Add(this.lblLootId);
            this.Controls.Add(this.panel1);
            this.Name = "EditLootDrop";
            this.Text = "EditLootDrop";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.Label lblLootId;
        private System.Windows.Forms.Button btnSave;
        private System.Windows.Forms.Label lblBoxInfo;
    }
}