import { isEmail } from "class-validator";
import { normalizeEmailForComparison } from "./email";
import { validatePasswordPolicy } from "./password-policy";

export interface ConfiguredRootAdmin {
  email: string;
  password: string;
}

export function getOptionalConfiguredRootAdminEmail(): string | undefined {
  const email = process.env.QUANTFLOW_ADMIN_EMAIL?.trim();
  return email ? normalizeEmailForComparison(email) : undefined;
}

export function getRequiredConfiguredRootAdminEmail(): string {
  const email = process.env.QUANTFLOW_ADMIN_EMAIL?.trim();
  if (!email) {
    throw new Error("QUANTFLOW_ADMIN_EMAIL must be configured");
  }
  if (!isEmail(email)) {
    throw new Error("QUANTFLOW_ADMIN_EMAIL must be a valid email address");
  }
  return normalizeEmailForComparison(email);
}

export function getRequiredConfiguredRootAdminPassword(): string {
  const password = process.env.QUANTFLOW_ADMIN_PASSWORD;
  if (password === undefined || password === "") {
    throw new Error("QUANTFLOW_ADMIN_PASSWORD must be configured");
  }
  return password;
}

export function getRequiredConfiguredRootAdmin(): ConfiguredRootAdmin {
  const email = getRequiredConfiguredRootAdminEmail();
  const password = getRequiredConfiguredRootAdminPassword();
  const error = validatePasswordPolicy(password);
  if (error) {
    throw new Error(`QUANTFLOW_ADMIN_PASSWORD is invalid: ${error}`);
  }

  return { email, password };
}

export function isConfiguredRootAdminEmail(email: string): boolean {
  const configuredEmail = getOptionalConfiguredRootAdminEmail();
  return Boolean(
    configuredEmail && normalizeEmailForComparison(email) === configuredEmail,
  );
}
