namespace WOAdmin
{
    partial class EditItemUpgradeData
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
            this.cbSelectWeapon = new System.Windows.Forms.ComboBox();
            this.btnSave = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.cbWeapType = new System.Windows.Forms.ComboBox();
            this.label2 = new System.Windows.Forms.Label();
            this.cbUpgradeId = new System.Windows.Forms.ComboBox();
            this.panel1.SuspendLayout();
            this.SuspendLayout();
            // 
            // panel1
            // 
            this.panel1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)));
            this.panel1.AutoScroll = true;
            this.panel1.Controls.Add(this.cbSelectWeapon);
            this.panel1.Location = new System.Drawing.Point(12, 48);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(693, 546);
            this.panel1.TabIndex = 0;
            // 
            // cbSelectWeapon
            // 
            this.cbSelectWeapon.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbSelectWeapon.FormattingEnabled = true;
            this.cbSelectWeapon.Location = new System.Drawing.Point(484, 0);
            this.cbSelectWeapon.Name = "cbSelectWeapon";
            this.cbSelectWeapon.Size = new System.Drawing.Size(153, 21);
            this.cbSelectWeapon.TabIndex = 7;
            this.cbSelectWeapon.SelectedIndexChanged += new System.EventHandler(this.cbSelectWeapon_SelectedIndexChanged);
            // 
            // btnSave
            // 
            this.btnSave.Location = new System.Drawing.Point(704, 5);
            this.btnSave.Name = "btnSave";
            this.btnSave.Size = new System.Drawing.Size(75, 37);
            this.btnSave.TabIndex = 2;
            this.btnSave.Text = "Save";
            this.btnSave.UseVisualStyleBackColor = true;
            this.btnSave.Click += new System.EventHandler(this.btnSave_Click);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(10, 15);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(34, 13);
            this.label1.TabIndex = 4;
            this.label1.Text = "Type:";
            // 
            // cbWeapType
            // 
            this.cbWeapType.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbWeapType.FormattingEnabled = true;
            this.cbWeapType.Location = new System.Drawing.Point(50, 12);
            this.cbWeapType.Name = "cbWeapType";
            this.cbWeapType.Size = new System.Drawing.Size(116, 21);
            this.cbWeapType.TabIndex = 3;
            this.cbWeapType.SelectedIndexChanged += new System.EventHandler(this.cbWeapType_SelectedIndexChanged);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(172, 15);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(51, 13);
            this.label2.TabIndex = 6;
            this.label2.Text = "Upgrade:";
            // 
            // cbUpgradeId
            // 
            this.cbUpgradeId.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbUpgradeId.FormattingEnabled = true;
            this.cbUpgradeId.Location = new System.Drawing.Point(229, 12);
            this.cbUpgradeId.Name = "cbUpgradeId";
            this.cbUpgradeId.Size = new System.Drawing.Size(102, 21);
            this.cbUpgradeId.TabIndex = 5;
            this.cbUpgradeId.SelectedIndexChanged += new System.EventHandler(this.cbUpgradeId_SelectedIndexChanged);
            // 
            // EditItemUpgradeData
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(881, 606);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.cbUpgradeId);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.cbWeapType);
            this.Controls.Add(this.btnSave);
            this.Controls.Add(this.panel1);
            this.Name = "EditItemUpgradeData";
            this.Text = "EditItemUpgradeData";
            this.Load += new System.EventHandler(this.EditItemUpgradeData_Load);
            this.Shown += new System.EventHandler(this.EditItemUpgradeData_Shown);
            this.panel1.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.Button btnSave;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.ComboBox cbWeapType;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.ComboBox cbUpgradeId;
        private System.Windows.Forms.ComboBox cbSelectWeapon;
    }
}